#!/usr/bin/env python
from contextlib import contextmanager
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import Iterator
import os
import unittest

@contextmanager
def chdir(path: Path | str) -> Iterator[None]:
    saved = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(saved)

def diff_report_from_files() -> str:
    common = Path('common').read_text().rstrip('\n')
    a_only = Path('a_only').read_text().rstrip('\n')
    b_only = Path('b_only').read_text().rstrip('\n')
    return '\n'.join([
        'common:', common,
        'a_only:', a_only,
        'b_only:', b_only,
    ])

def diff_report_from_lists(*,
    common: list[str],
    a_only: list[str],
    b_only: list[str],
) -> str:
    return '\n'.join([
        'common:', '\n'.join(sorted(common)),
        'a_only:', '\n'.join(sorted(a_only)),
        'b_only:', '\n'.join(sorted(b_only)),
    ])

class TestDiffDir(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.diffdir_bin = Path.cwd() / 'diffdir'

    def diffdir(self, a: Path | str, b: Path | str) -> str:
        os.system(f'{self.diffdir_bin} {a} {b}')
        return diff_report_from_files()

    def test_same_and_different(self) -> None:
        with TemporaryDirectory() as tmpdir, chdir(tmpdir):
            for d in ['a/dir1', 'b/dir1']:
                Path(d).mkdir(parents=True)
            for f in ['a/same', 'b/same']:
                Path(f).write_text('hithere')
            Path('a/dir1/different').write_text('a side')
            Path('b/dir1/different').write_text('and b side')
            got = self.diffdir('a', 'b')
            want = diff_report_from_lists(
                common=['dir1', 'same'],
                a_only=['dir1/different'],
                b_only=['dir1/different'],
            )
            self.assertMultiLineEqual(str(want), got)

    def test_same_size_but_different(self) -> None:
        with TemporaryDirectory() as tmpdir, chdir(tmpdir):
            Path('a').mkdir()
            Path('b').mkdir()
            Path('a/almost_same').write_text('a side')
            Path('b/almost_same').write_text('b side')
            got = self.diffdir('a', 'b')
            want = diff_report_from_lists(
                common=[],
                a_only=['almost_same'],
                b_only=['almost_same'],
            )
            self.assertMultiLineEqual(str(want), got)

    def test_no_files_on_a(self) -> None:
        with TemporaryDirectory() as tmpdir, chdir(tmpdir):
            Path('a').mkdir()
            Path('b').mkdir()
            Path('b/file1').write_text('file1')
            Path('b/file2').write_text('file2')
            got = self.diffdir('a', 'b')
            want = diff_report_from_lists(
                common=[],
                a_only=[],
                b_only=['file1', 'file2'],
            )
            self.assertMultiLineEqual(str(want), got)

    def test_no_files_on_b(self) -> None:
        with TemporaryDirectory() as tmpdir, chdir(tmpdir):
            Path('a').mkdir()
            Path('b').mkdir()
            Path('a/file1').write_text('file1')
            Path('a/file2').write_text('file2')
            got = self.diffdir('a', 'b')
            want = diff_report_from_lists(
                common=[],
                a_only=['file1', 'file2'],
                b_only=[],
            )
            self.assertMultiLineEqual(str(want), got)

    def test_deep_hierarchy(self) -> None:
        with TemporaryDirectory() as tmpdir, chdir(tmpdir):
            Path('a/1/2/3/4/5/6/7/8').mkdir(parents=True)
            Path('b/1/2/3/4').mkdir(parents=True)
            Path('a/1/2/3/4/5/6/7/8/file1').write_text('file1')
            Path('b/1/2/3/4/file1').write_text('file1')
            got = self.diffdir('a', 'b')
            want = diff_report_from_lists(
                common=['1', '1/2', '1/2/3', '1/2/3/4'],
                a_only=['1/2/3/4/5'],
                b_only=['1/2/3/4/file1'],
            )
            self.assertMultiLineEqual(str(want), got)


if __name__ == '__main__':
    unittest.main()

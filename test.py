#!/usr/bin/env python
import unittest
import os
from tempfile import TemporaryDirectory
from pathlib import Path
from contextlib import contextmanager
from dataclasses import dataclass
from typing import Iterator

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
        cls.diffdir = Path.cwd() / 'diffdir'

    def test_same_and_different(self) -> None:
        with TemporaryDirectory(delete=True) as tmpdir, chdir(tmpdir):  # TODO: remove delete
            #print(tmpdir)  # TODO: remove
            for d in ['a/dir1', 'b/dir1']:
                Path(d).mkdir(parents=True)
            for f in ['a/same', 'b/same']:
                Path(f).write_text('hithere')
            Path('a/dir1/different').write_text('a side')
            Path('b/dir1/different').write_text('and b side')
            os.system(f'{self.diffdir} a b')
            got = diff_report_from_files()
            want = diff_report_from_lists(
                common=['dir1', 'same'],
                a_only=['dir1/different'],
                b_only=['dir1/different'],
            )
            self.assertMultiLineEqual(str(want), got)

if __name__ == '__main__':
    unittest.main()

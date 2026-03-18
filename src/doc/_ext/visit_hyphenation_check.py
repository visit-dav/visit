from __future__ import annotations

import os
import pathlib
import subprocess
import sys
from typing import Any

from sphinx.errors import SphinxError


def _find_checker_script(confdir: pathlib.Path) -> pathlib.Path:
    for candidate in [confdir, *confdir.parents]:
        script = candidate / "fix_rst_hyphenation.py"
        if script.is_file():
            return script
    raise SphinxError(
        "Hyphenation check failed to locate fix_rst_hyphenation.py under the docs tree. "
        "Expected it at src/doc/fix_rst_hyphenation.py (or an ancestor of conf.py)."
    )


def _should_run(app: Any) -> bool:
    if not getattr(app.config, "visit_hyphenation_check", True):
        return False
    if os.environ.get("VISIT_SKIP_HYPHENATION_CHECK"):
        return False
    return True


def _run_check(app: Any) -> None:
    if not _should_run(app):
        return

    confdir = pathlib.Path(app.confdir).resolve()
    script = _find_checker_script(confdir)
    configured_root = getattr(app.config, "visit_hyphenation_check_root", None)
    root = confdir if configured_root is None else pathlib.Path(configured_root).resolve()

    result = subprocess.run(
        [sys.executable, str(script), "--check", str(root)],
        cwd=str(confdir),
        text=True,
        capture_output=True,
    )
    if result.returncode == 0:
        return

    output = (result.stdout or "").strip()
    hint = (
        f"Hyphenation consistency check failed.\n\n"
        f"Fix by running:\n"
        f"  {sys.executable} {script} {root}\n"
    )
    if output:
        hint += f"\nFiles needing updates:\n{output}\n"
    if result.stderr:
        hint += f"\nChecker stderr:\n{result.stderr.strip()}\n"
    raise SphinxError(hint)


def setup(app: Any) -> dict[str, Any]:
    app.add_config_value("visit_hyphenation_check", True, "env")
    app.add_config_value("visit_hyphenation_check_root", None, "env")
    app.connect("builder-inited", _run_check)
    return {"version": "1.0", "parallel_read_safe": True, "parallel_write_safe": True}

import json
import shutil
import subprocess
from enum import Enum
from pathlib import Path

import pytest


def run_workflow(osclipath: Path, suffix: str, is_labs: bool, verbose: bool = False, debug: bool = False):
    base_osw_path = Path("with_errors.osw").resolve()
    assert base_osw_path.is_file(), f"{base_osw_path=} is not found"

    osw = json.loads(base_osw_path.read_text())
    osw_path = base_osw_path.parent / f"with_errors_{suffix}.osw"
    runDir = base_osw_path.parent / f"run_{suffix}"
    osw["run_directory"] = str(runDir)
    if runDir.is_dir():
        shutil.rmtree(runDir)
    runDir.mkdir(exist_ok=False)
    with open(osw_path, "w") as f:
        json.dump(osw, fp=f, indent=2, sort_keys=True)

    # Fake having an in.idf or it won't run in the "classic" subcommand
    # Doing it for labs too so that it's less confusing
    with open(runDir / "in.idf", "w") as f:
        f.write("Building,;")
    command = [str(osclipath)]
    if not is_labs:
        command.append("classic")
    if verbose:
        command.append("--verbose")
    command += ["run", "--postprocess_only"]
    if debug:
        command.append("--debug")
    command += ["-w", str(osw_path)]
    # print(f"Running command: {' '.join(command)}")
    lines = subprocess.check_output(command, encoding="utf-8").splitlines()
    return runDir

#
@pytest.mark.parametrize(
    "is_labs",
    [pytest.param(True, id="labs"), pytest.param(False, id="classic")],
)
def test_run_log_debug(osclipath, is_labs: bool):
    suffix = "labs" if is_labs else "classic"
    suffix += "_debug"
    runDir = run_workflow(osclipath=osclipath, suffix=suffix, is_labs=is_labs, verbose=False, debug=True)
    assert runDir.is_dir()
    run_log_path = runDir / "run.log"
    assert run_log_path.is_file()
    run_log = run_log_path.read_text()
    assert "runner.registerError called" in run_log

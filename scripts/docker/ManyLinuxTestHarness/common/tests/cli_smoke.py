import os

expected_version = os.environ["VISIT_VERSION"]
actual_version = Version()

if actual_version != expected_version:
    raise RuntimeError(
        "Expected VisIt version %s, got %s" % (expected_version, actual_version)
    )

print("VISIT_CLI_SMOKE_OK version=%s" % actual_version)

import pytest
import requests
from test_runner import (
    run,
    start_server,
    stop_server,
    wait_for_websocket_download_success_event,
)


class TestCliEngineUninstall:

    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        stop_server()

    @pytest.mark.asyncio
    async def test_engines_uninstall_llamacpp_should_be_successfully(self):
        requests.post("http://127.0.0.1:3928/v1/engines/llama-cpp")
        await wait_for_websocket_download_success_event(timeout=None)
        exit_code, output, error = run(
            "Uninstall engine", ["engines", "uninstall", "llama-cpp"]
        )
        assert "Engine llama-cpp uninstalled successfully!" in output
        assert exit_code == 0, f"Install engine failed with error: {error}"

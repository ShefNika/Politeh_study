import pytest
from MyString import MyString
from controller.controller_editor import EditorController
from model.text_model import EditorModel


class TestEditorController:
    @pytest.fixture
    def controller_setup(self, mocker):
        mock_adapter = mocker.Mock()
        mock_adapter.get_key = mocker.Mock()
        mock_adapter.get_window_size = mocker.Mock(return_value=(25, 80))
        mock_adapter.RIGHT = 261
        mock_adapter.LEFT = 260
        mock_adapter.UP = 259
        mock_adapter.DOWN = 258
        mock_adapter.BACKSPACE = 263
        mock_adapter.ENTER = 10
        mock_adapter.PPAGE = 339
        mock_adapter.NPAGE = 338
        model = EditorModel()
        model.screen_width = 80
        controller = EditorController(mock_adapter, model)
        return controller, mock_adapter, model

    def test_normal_mode_movement(self, controller_setup):
        controller, mock_adapter, model = controller_setup
        model.mode = "INSERT"
        controller.handle_key(ord('h'))
        controller.handle_key(ord('e'))
        controller.handle_key(ord('l'))
        controller.handle_key(ord('l'))
        controller.handle_key(ord('o'))
        model.mode = "NORMAL"
        controller.handle_key(mock_adapter.LEFT)
        assert model.cursor_x == 4
        controller.handle_key(mock_adapter.RIGHT)
        assert model.cursor_x == 5

    def test_mode_transitions(self, controller_setup):
        controller, mock_adapter, model = controller_setup

        controller.handle_key(ord(':'))
        assert model.mode == "COMMAND"

        controller.handle_key(27)  # ESC
        assert model.mode == "NORMAL"

        controller.handle_key(ord('i'))
        assert model.mode == "INSERT"

        controller.handle_key(27)
        assert model.mode == "NORMAL"

    def test_command_execution(self, controller_setup, mocker):
        controller, mock_adapter, model = controller_setup
        mock_execute = mocker.patch('controller.controller_editor.CommandInvoker.execute')

        model.mode = "COMMAND"
        model.command_buffer = mocker.Mock()
        model.command_buffer.c_str.return_value = "w"

        controller.execute_command("w")
        assert mock_execute.called

    def test_insert_mode_typing(self, controller_setup):
        controller, mock_adapter, model = controller_setup
        model.mode = "INSERT"
        controller.handle_key(ord('a'))
        controller.handle_key(ord('b'))
        controller.handle_key(ord('c'))

        assert model.lines[0].c_str() == "abc"
        assert model.cursor_x == 3

        controller.handle_key(10)
        assert len(model.lines) == 2
        assert model.cursor_y == 1

    def test_search_mode(self, controller_setup, mocker):
        controller, mock_adapter, model = controller_setup
        model.lines = [MyString("test content here"), MyString("more test here")]
        model.cursor_x = 0
        model.cursor_y = 0
        controller.handle_key(ord('/'))
        assert model.mode == "SEARCH"
        assert model.search_direction == 1
        controller.handle_key(ord('t'))
        controller.handle_key(ord('e'))
        controller.handle_key(ord('s'))
        controller.handle_key(ord('t'))
        assert model.search_pattern.c_str() == "test"
        mock_search = mocker.patch.object(model, '_search_forward', return_value=True)
        controller.handle_key(10)
        assert mock_search.called
        assert model.mode == "NORMAL"

    def test_backspace_in_insert_mode(self, controller_setup):
        controller, mock_adapter, model = controller_setup
        model.mode = "INSERT"
        controller.handle_key(ord('a'))
        controller.handle_key(ord('b'))
        controller.handle_key(ord('c'))
        controller.handle_key(mock_adapter.BACKSPACE)
        assert model.lines[0].c_str() == "ab"
        assert model.cursor_x == 2


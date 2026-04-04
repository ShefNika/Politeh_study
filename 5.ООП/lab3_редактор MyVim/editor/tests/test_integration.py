import pytest
from controller.controller_editor import EditorController
from view.view_editor import EditorView
from model.text_model import EditorModel


class TestIntegration:
    @pytest.fixture
    def full_system(self, mocker):
        mock_adapter = mocker.Mock()
        mock_adapter.get_key = mocker.Mock(side_effect=[
            ord('i'),
            ord('h'),
            ord('e'),
            ord('l'),
            ord('l'),
            ord('o'),
            27,
            ord(':'),
            ord('w'),
            10,
            27
        ])
        mock_adapter.get_window_size = mocker.Mock(return_value=(25, 80))
        mock_adapter.RIGHT = 261
        mock_adapter.LEFT = 260
        mock_adapter.clear = mocker.Mock()
        mock_adapter.addstr = mocker.Mock()
        mock_adapter.draw_status_bar = mocker.Mock()
        mock_adapter.move = mocker.Mock()
        mock_adapter.refresh = mocker.Mock()
        model = EditorModel()
        model.screen_width = 80
        view = EditorView(model, mock_adapter)
        controller = EditorController(mock_adapter, model)
        return controller, model, view, mock_adapter

    def test_full_edit_cycle(self, full_system):
        controller, model, view, mock_adapter = full_system
        for _ in range(7):  # i, h, e, l, l, o, ESC
            key = mock_adapter.get_key()
            controller.handle_key(key)

        assert model.lines[0].c_str() == "hello"
        assert model.mode == "NORMAL"
        assert mock_adapter.refresh.called

    def test_file_open_save_cycle(self, full_system, mocker, tmp_path):
        controller, model, view, mock_adapter = full_system
        test_file = tmp_path / "test.txt"
        test_file.write_text("Test content\nSecond line")
        mock_open = mocker.patch('controller.controller_editor.OpenFile')
        mock_save = mocker.patch('controller.controller_editor.SaveFile')
        controller.execute_command(f"o {test_file}")
        assert mock_open.called
        model.mode = "INSERT"
        controller.handle_key(ord('!'))
        controller.handle_key(27)
        controller.execute_command("w")
        assert mock_save.called

    def test_observer_pattern_integration(self, full_system):
        controller, model, view, mock_adapter = full_system
        mock_adapter.clear.reset_mock()
        mock_adapter.refresh.reset_mock()
        model.mode = 'INSERT'
        model.insert_char('X')

        assert mock_adapter.clear.called
        assert mock_adapter.refresh.called


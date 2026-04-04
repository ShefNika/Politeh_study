import pytest
from MyString import MyString
from view.view_editor import EditorView
from model.text_model import EditorModel


class TestEditorView:
    @pytest.fixture
    def view_setup(self, mocker):
        model = EditorModel()
        model.screen_width = 80
        mock_adapter = mocker.Mock()
        mock_adapter.get_window_size = mocker.Mock(return_value=(20, 80))
        mock_adapter.clear = mocker.Mock()
        mock_adapter.addstr = mocker.Mock()
        mock_adapter.draw_status_bar = mocker.Mock()
        mock_adapter.move = mocker.Mock()
        mock_adapter.refresh = mocker.Mock()
        mock_adapter.has_colors = mocker.Mock(return_value=True)
        mock_adapter.color_pair = mocker.Mock(return_value=1)

        view = EditorView(model, mock_adapter)
        return view, model, mock_adapter

    def test_update_normal_mode(self, view_setup):
        view, model, mock_adapter = view_setup
        model.mode = "INSERT"
        model.insert_char('H')
        model.insert_char('i')
        assert mock_adapter.clear.called
        assert mock_adapter.addstr.called
        assert mock_adapter.draw_status_bar.called
        assert mock_adapter.refresh.called
        call_args = mock_adapter.draw_status_bar.call_args[0][0]
        assert "INSERT" in call_args
        assert "1/1" in call_args

    def test_update_command_mode(self, view_setup, mocker):
        view, model, mock_adapter = view_setup
        model.mode = "COMMAND"
        mock_buffer = mocker.Mock()
        mock_buffer.c_str = mocker.Mock(return_value="w")
        model.command_buffer = mock_buffer

        view.update()

        call_args = mock_adapter.draw_status_bar.call_args[0][0]
        assert "COMMAND" in call_args
        assert ":w_" in call_args

    def test_update_search_mode(self, view_setup, mocker):
        view, model, mock_adapter = view_setup
        model.mode = "SEARCH"
        model.search_direction = 1
        mock_pattern = mocker.Mock()
        mock_pattern.c_str = mocker.Mock(return_value="test")
        model.search_pattern = mock_pattern

        view.update()

        call_args = mock_adapter.draw_status_bar.call_args[0][0]
        assert "SEARCH" in call_args
        assert "/test_" in call_args

    def test_update_help_mode(self, view_setup, mocker):
        view, model, mock_adapter = view_setup
        model.mode = "HELP"
        mock_line1 = mocker.Mock()
        mock_line1.size = mocker.Mock(return_value=20)
        mock_line1.substr = mocker.Mock(return_value=mocker.Mock(c_str=mocker.Mock(return_value="HELP TEXT")))
        mock_line2 = mocker.Mock()
        mock_line2.size = mocker.Mock(return_value=25)
        mock_line2.substr = mocker.Mock(return_value=mocker.Mock(c_str=mocker.Mock(return_value="More help")))

        model.help_text = [mock_line1, mock_line2]

        view.update()

        assert mock_adapter.clear.called
        assert mock_adapter.draw_status_bar.called

        call_args = mock_adapter.draw_status_bar.call_args[0][0]
        assert "HELP" in call_args
        assert "Press ESC to exit" in call_args or "HELP" in call_args

    def test_calculate_screen_position(self, view_setup):
        view, model, mock_adapter = view_setup
        for _ in range(5):
            model.insert_char('x')
        model.new_line_below()
        model.insert_char('y')
        mock_adapter.get_window_size.return_value = (5, 10)
        y, x = view.calculate_screen_position()
        assert isinstance(y, int)
        assert isinstance(x, int)
        assert 0 <= y < 5
        assert 0 <= x < 10

    def test_syntax_highlighting(self, view_setup, mocker):
        view, model, mock_adapter = view_setup
        model.syntax_highlight = True
        model.lines = [MyString("def test(): # comment")]
        mock_highlighter = mocker.Mock()
        mock_highlighter.highlight_line = mocker.Mock(return_value=[
            (0, 3, 1),  # 'def' - ключевое слово
            (12, 21, 3)  # комментарий
        ])
        mock_highlighter.get_color = mocker.Mock(return_value=1)
        view.highlighter = mock_highlighter

        view.update()

        assert mock_adapter.addstr.called
        color_calls = []
        for call in mock_adapter.addstr.call_args_list:
            if len(call[0]) > 3:
                color_calls.append(call)
        assert len(color_calls) > 0

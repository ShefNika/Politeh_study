import pytest
from model.text_model import EditorModel
from MyString import MyString


class TestEditorModel:

    @pytest.fixture
    def setup(self):
        model = EditorModel()
        model.screen_width = 80
        return model

    def test_initial_state(self, setup):
        model = setup
        assert len(model.lines) == 1
        assert model.lines[0].c_str() == ""
        assert model.cursor_x == 0
        assert model.cursor_y == 0
        assert model.mode == "NORMAL"
        assert not model.modified

    def test_insert_char(self, setup):
        model = setup
        model.insert_char('a')
        assert model.lines[0].c_str() == "a"
        assert model.cursor_x == 1
        assert model.modified == True

    def test_move_cursor(self, setup):
        model = setup
        model.insert_char('a')
        model.insert_char('b')
        model.insert_char('c')

        model.move_cursor(dx=-3)
        model.move_cursor(dx=1)
        assert model.cursor_x == 1

        model.new_line_below()
        model.move_cursor(dy=1)
        assert model.cursor_y == 1
        model.move_cursor(dy=-1)
        assert model.cursor_y == 0

    def test_new_line_below(self, setup):
        model = setup
        model.insert_char('h')
        model.insert_char('i')
        model.cursor_x = 1

        model.new_line_below()

        assert len(model.lines) == 2
        assert model.lines[0].c_str() == "h"
        assert model.lines[1].c_str() == "i"
        assert model.cursor_y == 1
        assert model.cursor_x == 0

    def test_backspace(self, setup):
        model = setup
        model.insert_char('a')
        model.insert_char('b')

        model.backspace()
        assert model.lines[0].c_str() == "a"
        assert model.cursor_x == 1

        model.new_line_below()
        model.backspace()
        assert len(model.lines) == 1
        assert model.cursor_y == 0

    def test_cut_line(self, setup):
        model = setup
        model.lines = [MyString("line1"), MyString("line2")]
        model.cursor_y = 0

        model.cut_line()

        assert len(model.lines) == 1
        assert model.lines[0].c_str() == "line2"
        assert len(model.clipboard) == 1
        assert model.clipboard[0].c_str() == "line1"

    def test_search_functionality(self, setup):
        model = setup
        model.lines = [MyString("find me here"), MyString("and here too")]
        model.search_pattern = MyString("here")
        model.search_direction = 1
        model.cursor_x = 0
        model.cursor_y = 0

        result = model._search_forward()
        assert result == True
        assert model.cursor_x == 8
        assert model.cursor_y == 0

        result = model._search_forward()
        assert result == True
        assert model.cursor_y == 1

    def test_undo_functionality(self, mocker, setup):
        model = setup
        mock_history_instance = mocker.Mock()
        mock_history_instance.get_last_command.return_value = (
            mocker.Mock(__class__=mocker.Mock(__name__='TestCommand')),
            {
                'lines': [MyString("old")],
                'cursor_x': 0,
                'cursor_y': 0,
                'modified': False
            }
        )
        mock_history_instance.commands = ['dummy']
        mock_history_instance.states = ['dummy']
        mock_history_class = mocker.Mock(return_value=mock_history_instance)
        mocker.patch('model.text_model.CommandHistory', new=mock_history_class)
        mock_restore = mocker.patch.object(model, 'restore_state')
        model.undo_last_command()
        assert mock_restore.called
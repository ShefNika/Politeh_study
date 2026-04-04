import unittest
from unittest.mock import Mock, patch
from pop import GameState, HangmanGame, View, User, HangmanFigure, IView, IUser, \
    AnimalsCategory


class TestGameState(unittest.TestCase):
    """Тестирование функционала класса GameState"""
    def setUp(self):
        self.state = GameState()
        self.state.word = 'test'
        self.state.word_split = list(self.state.word)

    def test_game_state(self):
        # Тестирование check_guess c  неправильной буквой
        self.assertFalse(self.state.check_guess('x'))
        self.assertEqual(self.state.chances, 18)
        self.assertIn('x', self.state.wrong_letters)

        # Тестирование check_guess c  правильной буквой
        self.assertTrue(self.state.check_guess('t'))
        self.assertIn('t', self.state.guessed_letters)
        self.assertEqual(self.state.chances, 18)

        # Тестирование is_win
        self.state.check_guess('e')
        self.state.check_guess('s')
        self.assertTrue(self.state.is_win())

        # Тестирование is_lose
        self.state.reset()
        self.state.chances = 1
        self.state.check_guess('y')
        self.assertTrue(self.state.is_lose())
        self.assertEqual(self.state.chances, 0)

class TestHangmanGame(unittest.TestCase):
    """Тестирование функционала класса HangmanGame"""
    def setUp(self):
        self.game = HangmanGame()

    @patch('random.choice', return_value='cow')  # Mock для фиксированного слова
    @patch('pop.timer', side_effect=[100.0, 150.0, 200.0])  # Mock для start/pause/unpause
    def test_hangman_game(self, mock_timer, mock_random):
        mock_observer = Mock() # Mock как наблюдатель
        # Тестирование методов работы с наблюдателем
        self.game.attach(mock_observer)

        # Тестирование select_category
        self.game.select_category("Animals")
        self.assertEqual(self.game.state.word, 'cow')
        self.assertFalse(self.game.state.in_menu)
        self.assertEqual(self.game.state.start_time, 100.0)
        mock_observer.update.assert_called_once()

        # Тестирование guess_letter (стандартный случай)
        self.game.guess_letter('c')
        self.assertIn('c', self.game.state.guessed_letters)
        mock_observer.update.assert_called()

        # Тестирование pause
        self.game.pause()
        self.assertTrue(self.game.state.in_pause)
        self.assertEqual(self.game.state.pause_start, 150.0)
        mock_observer.update.assert_called()

        # Тестирование unpause
        self.game.unpause()
        self.assertFalse(self.game.state.in_pause)
        self.assertEqual(self.game.state.start_time, 150.0)
        mock_observer.update.assert_called()

class TestView(unittest.TestCase):
    """Тестирование функционала класса View"""

    def setUp(self):
        self.mock_view_impl = Mock(spec=IView)
        self.mock_game = Mock(spec=HangmanGame)
        self.mock_game.state = GameState()
        self.mock_game.hangman_figure = Mock(spec=HangmanFigure)
        self.view = View(self.mock_view_impl, self.mock_game)

    def test_view(self):
        # Тестирование render_menu
        self.mock_game.state.in_menu = True
        self.mock_view_impl.get_mouse_pos.return_value = (0, 0)
        self.view.update()
        self.mock_view_impl.draw_rect.assert_any_call(View.BLACK, (150, 450, 150, 100))
        self.mock_view_impl.draw_rect.assert_any_call(View.BLACK, (550, 450, 150, 100))
        self.mock_view_impl.draw_rect.assert_any_call(View.BLACK, (150, 50, 150, 100))
        self.mock_view_impl.draw_rect.assert_any_call(View.BLACK, (550, 50, 150, 100))
        self.mock_view_impl.fill_screen.assert_called_with(View.WHITE)
        self.mock_view_impl.draw_text.assert_called()
        self.mock_view_impl.update_display.assert_called()
        self.mock_view_impl.get_mouse_pos.return_value = (200, 500)
        self.view.update()
        self.mock_view_impl.draw_rect.assert_any_call(View.LIGHTGREY, (150, 450, 150, 100))

        # Тестирование render_game
        self.mock_view_impl.get_mouse_pos.return_value = (0, 0)
        self.mock_game.state.in_menu = False
        self.mock_game.state.in_game = True
        self.mock_game.state.word = 'test'
        self.mock_game.state.word_split = list('test')
        self.mock_game.state.selected_category = AnimalsCategory()
        self.view.update()
        self.mock_view_impl.draw_text.assert_any_call("Animals", 40, View.BLACK, (400, 50))
        self.mock_view_impl.draw_text.assert_any_call("_", 40, View.BLACK, (160, 200))
        self.mock_view_impl.draw_rect.assert_any_call(View.BLACK, (100, 300, 250, 250), 2)

        # Тестирование render_pause
        self.mock_game.state.in_pause = True
        self.view.update()
        self.mock_view_impl.draw_text.assert_any_call("Paused", 115, View.BLACK, (400, 300), center=True)
        self.mock_view_impl.draw_rect.assert_any_call(View.DARKLIGHTBLUE, (550, 450, 100, 50))

        # Тестирование render_end
        self.mock_game.state.in_pause = False
        self.mock_game.state.in_game = False
        self.mock_game.state.in_end = True
        self.mock_game.state.win = True
        self.view.update()
        self.mock_view_impl.draw_text.assert_any_call("You win!", 100, View.DARKLIGHTRED, (400, 100))
        self.mock_view_impl.draw_rect.assert_any_call(View.DARKLIGHTRED, (450, 450, 100, 50))


class TestUser(unittest.TestCase):
    """Тестирование функционала класса User"""

    def setUp(self):
        self.mock_user_impl = Mock(spec=IUser)
        self.mock_game = Mock(spec=HangmanGame)
        self.mock_state = Mock()
        self.mock_game.state = self.mock_state
        self.user = User(self.mock_user_impl, self.mock_game)

    def test_user(self):
        # Тестирование handle_input - нажатие клавиши с буквой
        self.mock_user_impl.get_key_events.return_value = ['a']
        self.mock_user_impl.get_mouse_click.return_value = (False, False, False)
        self.mock_game.state.in_game = True
        self.mock_game.state.in_pause = False
        self.user.handle_input()
        self.mock_game.guess_letter.assert_called_with('a')

        # Тестирование handle_input - нажатие пробела
        self.mock_user_impl.get_key_events.return_value = ['space']
        self.user.handle_input()
        self.mock_game.pause.assert_called_once()

        # Тестирование handle_input - нажатие escape
        self.mock_user_impl.get_key_events.return_value = ['escape']
        self.user.handle_input()
        self.mock_game.end_game.assert_called_once()

        # Тестирование handle_input - клик мыши в меню
        self.mock_user_impl.get_key_events.return_value = []
        self.mock_user_impl.get_mouse_pos.return_value = (200, 500)
        self.mock_user_impl.get_mouse_click.return_value = (True, False, False)
        self.mock_game.state.in_menu = True
        self.user.handle_input()
        self.mock_game.select_category.assert_called_with("Animals")

        # Тестирование handle_input - клик мыши в паузе
        self.mock_game.state.in_menu = False
        self.mock_game.state.in_game = False
        self.mock_game.state.in_pause = True
        self.mock_user_impl.get_mouse_pos.return_value = (200, 475)
        self.mock_user_impl.get_mouse_click.return_value = (True, False, False)
        self.user.handle_input()
        self.mock_game.unpause.assert_called_once()

        # Тестирование handle_input - клик мыши в завершении
        self.mock_game.state.in_menu = False
        self.mock_game.state.in_game = False
        self.mock_game.state.in_pause = False
        self.mock_game.state.in_end = True
        self.mock_user_impl.get_mouse_pos.return_value = (460, 460)
        self.mock_user_impl.get_mouse_click.return_value = (True, False, False)
        self.user.handle_input()
        self.mock_game.back_to_menu.assert_called_once()


class TestHangmanFigure(unittest.TestCase):
    """Тестирование функционала класса HangmanFigure"""

    def setUp(self):
        self.figure = HangmanFigure()
        self.mock_view = Mock(spec=IView)

    def test_hangman_figure(self):
        # Тестирование draw с 0 ошибок
        self.figure.draw(self.mock_view, 0)
        self.assertEqual(len(self.mock_view.method_calls), 0)

        # Тестирование draw с 5 ошибками
        self.figure.draw(self.mock_view, 5)
        self.assertEqual(self.mock_view.draw_rect.call_count, 5)
        self.mock_view.draw_rect.assert_any_call((0, 0, 0), (450, 550, 100, 10))  # Первая

        # Тестирование draw с 13 ошибками
        self.figure.draw(self.mock_view, 13)
        self.assertTrue(self.mock_view.draw_line.called)
        self.assertTrue(self.mock_view.draw_circle.called)
        self.mock_view.draw_circle.assert_called_with((0, 0, 0), (605, 325), 30)


if __name__ == '__main__':
    unittest.main()
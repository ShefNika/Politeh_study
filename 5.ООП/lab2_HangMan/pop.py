import sys
from abc import ABC, abstractmethod
import random
from typing import List, Tuple, Optional, Dict, Set
from timeit import default_timer as timer

import pygame

class IObserver(ABC):
    """
    Интерфейс наблюдателя
    """
    @abstractmethod
    def update(self) -> None:
        """
        Метод, вызываемый при уведомлении от HangmanGame
        """
        pass

class WordCategory:
    words: List[str] = []
    def get_random_word(self) -> str:
        """
        Получить случайное слово из категории для игры
        :return: слово из категории
        """
        return random.choice(self.words)
    def get_title(self) -> str:
        """
        Получить название категории
        :return: название категории
        """
        class_name = self.__class__.__name__
        return class_name[:-8]

class AnimalsCategory(WordCategory):
    words = ['cow', 'dog', 'cat', 'pig', 'zebra', 'bird', 'giraffe', 'lion', 'tiger', 'penguin', 'hamster', 'fox',
             'panda', 'bear', 'cheetah', 'ostrich', 'meerkat', 'whale', 'shark', 'horse', 'monkey', 'octopus',
             'kitten', 'kangaroo', 'chicken', 'fish', 'rabbit', 'sheep']

class VehiclesCategory(WordCategory):
    words = ['car', 'bus', 'train', 'airplane', 'plane', 'ship', 'jet', 'boat', 'lorry', 'tractor', 'bike',
             'motorbike', 'tram', 'van', 'ambulance', 'fire engine', 'rocket', 'taxi', 'caravan', 'coach', 'lorry',
             'scooter', 'sleigh', 'tank', 'wagon', 'spaceship']


class FoodCategory(WordCategory):
    words = ['apple', 'banana', 'orange', 'peach', 'pizza', 'donut', 'chips', 'sandwich', 'cookie', 'cucumber', 'carrot',
            'sweetcorn', 'ice cream', 'pancake', 'bread', 'potato', 'tomato', 'nuts', 'yogurt', 'pasta', 'rice',
            'cheese', 'soup', 'fish', 'egg', 'meat', 'ham', 'sausage']

class SportCategory(WordCategory):
    words = ['rugby', 'football', 'netball', 'basketball', 'swimming', 'hockey', 'curling', 'running', 'golf', 'tennis',
             'badmington', 'archery', 'volleyball', 'bowling', 'dancing', 'gym', 'skating', 'baseball', 'rounders',
             'boxing', 'climbing', 'canoe', 'cycling', 'fencing', 'karate', 'shooting', 'cricket']


class IView(ABC):
    @abstractmethod
    def init_screen(self, title: str, width: int = 800, height: int = 600) -> None:
        pass

    @abstractmethod
    def draw_rect(self, color: Tuple[int, int, int], position: Tuple[int, int, int, int], border: int = 0) -> None:
        pass

    @abstractmethod
    def draw_line(self, color: Tuple[int, int, int], start: Tuple[int, int], end: Tuple[int, int], width: int) -> None:
        pass

    @abstractmethod
    def draw_circle(self, color: Tuple[int, int, int], center: Tuple[int, int], radius: int) -> None:
        pass

    @abstractmethod
    def draw_text(self, text: str, font_size: int, color: Tuple[int, int, int], position: Tuple[int, int], center: bool = True) -> None:
        pass

    @abstractmethod
    def update_display(self) -> None:
        pass

    @abstractmethod
    def fill_screen(self, color: Tuple[int, int, int]) -> None:
        pass

    @abstractmethod
    def tick(self, fps: int) -> None:
        pass

    @abstractmethod
    def quit(self) -> None:
        pass

    @abstractmethod
    def get_mouse_pos(self) -> Tuple[int, int]:
        pass

class IUser(ABC):
    @abstractmethod
    def get_mouse_pos(self) -> Tuple[int, int]:
        pass

    @abstractmethod
    def get_mouse_click(self) -> Tuple[bool, bool, bool]:
        pass

    @abstractmethod
    def get_key_events(self) -> List[str]:
        pass

class PygameRenderer(IView, IUser):
    """
    Класс - реализация рендеринга и ввода с помощью pygame
    """
    def __init__(self, width: int = 800, height: int = 600, fps: int = 30) -> None:
        pygame.init()
        self.screen: Optional[pygame.Surface] = None
        self.clock: pygame.time.Clock = pygame.time.Clock()
        self.font_cache: Dict[int, pygame.font.Font] = {}  # словарь {размер текста, шрифт}
        self.width: int = width
        self.height: int = height
        self.fps: int = fps

    def get_font(self, size: int) -> pygame.font.Font:
        """
        Получить шрифт
        :param size: размер шрифта
        :return: объект шрифта
        """
        if size not in self.font_cache:
            self.font_cache[size] = pygame.font.Font("freesansbold.ttf", size)
        return self.font_cache[size]

    def init_screen(self, title: str, width: int = 800, height: int = 600) -> None:
        """
          Инициализировать экран
          :param width: ширина экрана
          :param height: высота экрана
          :param title: заголовок окна
          """
        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption(title)

    def draw_rect(self, color: Tuple[int, int, int], position: Tuple[int, int, int, int], border: int = 0) -> None:
        """
        Нарисовать прямоугольник
        :param color: цвет (RGB)
        :param position: координаты верхнего левого угла и размеры (x, y, width, height)
        :param border: толщина границы (по умолчанию 0 - заливка)
        """
        if self.screen:
            pygame.draw.rect(self.screen, color, position, border)

    def draw_line(self, color: Tuple[int, int, int], start: Tuple[int, int], end: Tuple[int, int], width: int) -> None:
        """
        Нарисовать линию
        :param color: цвет (RGB)
        :param start: начальные координаты (x, y)
        :param end: конечные координаты (x, y)
        :param width: толщина линии
        """
        if self.screen:
            pygame.draw.line(self.screen, color, start, end, width)

    def draw_circle(self, color: Tuple[int, int, int], center: Tuple[int, int], radius: int) -> None:
        """
        Нарисовать круг
        :param color: цвет (RGB)
        :param center: координаты центра
        :param radius: радиус
        """
        if self.screen:
            pygame.draw.circle(self.screen, color, center, radius)

    def draw_text(self, text: str, font_size: int, color: Tuple[int, int, int], position: Tuple[int, int], center: bool = True) -> None:
        """
        Нарисовать текст
        :param text: текст для отображения
        :param font_size: размер шрифта
        :param color: цвет (RGB)
        :param position: координаты (x, y)
        :param center: выравнивание по центру (по умолчанию True)
        """
        if self.screen:
            font = self.get_font(font_size)
            text_surf = font.render(text, True, color)  # True - сглаживание
            text_rect = text_surf.get_rect()  # Прямоугольник для текстовой позиции
            if center:
                text_rect.center = position
            else:
                text_rect.topleft = position
            self.screen.blit(text_surf, text_rect)

    def get_mouse_pos(self) -> Tuple[int, int]:
        """
        Получить положение мыши
        :return: кортеж (x, y) текущих координат мыши
        """
        return pygame.mouse.get_pos()

    def get_mouse_click(self) -> Tuple[bool, bool, bool]:
        """
        Получить состояние кликов мыши
        :return: кортеж (left, middle, right) состояния кнопок мыши
        """
        pressed = pygame.mouse.get_pressed()
        return (pressed[0] == 1, pressed[1] == 1, pressed[2] == 1)

    def get_key_events(self) -> List[str]:
        """
        Получить события клавиатуры
        :return: список нажатых клавиш (строки)
        """
        keys = []
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    keys.append('space')
                elif event.key == pygame.K_ESCAPE:
                    keys.append('escape')
                elif pygame.K_a <= event.key <= pygame.K_z:
                    keys.append(chr(event.key))
        return keys

    def update_display(self) -> None:
        """
       Обновить дисплей
       """
        pygame.display.update()

    def fill_screen(self, color: Tuple[int, int, int]) -> None:
        """
        Заполнить экран цветом
        :param color: цвет (RGB)
        """
        if self.screen:
            self.screen.fill(color)

    def tick(self, fps: int) -> None:
        """
        Установить частоту кадров
        :param fps: частота кадров в секунду
        """
        self.clock.tick(fps if fps else self.fps)

    def quit(self) -> None:
        """
        Завершить приложение
        """
        pygame.quit()
        sys.exit()

class HangmanFigure:
    """
    Класс для рисования фигуры виселицы
    """
    def __init__(self) -> None:
        self.max_parts: int = 19

    def draw(self, view: IView, errors: int) -> None:
        """
        Нарисовать фигуру виселицы в зависимости от количества ошибок
        :param view: объект класса view
        :param errors: количество ошибок
        """
        if errors >= 1:
            view.draw_rect((0, 0, 0), (450, 550, 100, 10))
        if errors >= 2:
            view.draw_rect((0, 0, 0), (550, 550, 100, 10))
        if errors >= 3:
            view.draw_rect((0, 0, 0), (650, 550, 100, 10))
        if errors >= 4:
            view.draw_rect((0, 0, 0), (500, 450, 10, 100))
        if errors >= 5:
            view.draw_rect((0, 0, 0), (500, 350, 10, 100))
        if errors >= 6:
            view.draw_rect((0, 0, 0), (500, 250, 10, 100))
        if errors >= 7:
            view.draw_rect((0, 0, 0), (500, 250, 150, 10))
        if errors >= 8:
            view.draw_rect((0, 0, 0), (600, 250, 100, 10))
        if errors >= 9:
            view.draw_rect((0, 0, 0), (600, 250, 10, 50))
        if errors >= 10:
            view.draw_line((0, 0, 0), (505, 505), (550, 550), 10)
        if errors >= 11:
            view.draw_line((0, 0, 0), (550, 250), (505, 295), 10)
        if errors >= 12:
            view.draw_line((0, 0, 0), (505, 505), (460, 550), 10)
        if errors >= 13:
            view.draw_circle((0, 0, 0), (605, 325), 30)
        if errors >= 14:
            view.draw_rect((0, 0, 0), (600, 350, 10, 60))
        if errors >= 15:
            view.draw_rect((0, 0, 0), (600, 410, 10, 60))
        if errors >= 16:
            view.draw_line((0, 0, 0), (605, 375), (550, 395), 10)
        if errors >= 17:
            view.draw_line((0, 0, 0), (605, 375), (650, 395), 10)
        if errors >= 18:
            view.draw_line((0, 0, 0), (605, 465), (550, 485), 10)
        if errors >= 19:
            view.draw_line((0, 0, 0), (605, 465), (650, 485), 10)

class GameState:
    """
    Класс для хранения всех данных об игре
    """
    def __init__(self) -> None:
        """
        Инициализировать состояние игры
        """
        self._initialize()

    def reset(self) -> None:
        """
        Сбросить состояние игры
        """
        self._initialize()

    def _initialize(self):
        self.guessed_letters: Set[str] = set()
        self.chances: int = 19
        self.word: str = ''
        self.word_split: List[str] = []
        self.start_time: float = 0.0
        self.end_time: float = 0.0
        self.pause_start: float = 0.0
        self.wrong_letters: List[str] = []
        self.selected_category: Optional[WordCategory] = None
        self.in_menu: bool = True
        self.in_game: bool = False
        self.in_pause: bool = False
        self.in_end: bool = False
        self.win: bool = False
    def check_guess(self, letter: str) -> bool:
        """
        Проверить угаданную букву
        :param letter: буква для проверки
        :return: True, если буква угадана, иначе False
        """
        if letter in self.guessed_letters:
            return letter in self.word
        self.guessed_letters.add(letter)
        if letter in self.word:
            return True
        else:
            self.chances -= 1
            self.wrong_letters.append(letter)
            return False

    def is_win(self) -> bool:
        """
        Проверить победу
        :return: True, если все буквы угаданы
        """
        return all(char in self.guessed_letters for char in self.word)

    def is_lose(self) -> bool:
        """
        Проверить поражение
        :return: True, если шансы исчерпаны, иначе False
        """
        return self.chances == 0

    def get_time_taken(self) -> float:
        """
        Получить время игры
        :return: время в секундах
        """
        return self.end_time - self.start_time

class HangmanGame:
    """
    Класс реализации логики игры
    """
    def __init__(self) -> None:
        """
        Инициализировать игру
        """
        self.state: GameState = GameState()
        self.categories: Dict[str, WordCategory] = {
            "Animals": AnimalsCategory(),
            "Vehicles": VehiclesCategory(),
            "Foods": FoodCategory(),
            "Sports": SportCategory()
        }
        self.observers: List[IObserver] = []
        self.hangman_figure: HangmanFigure = HangmanFigure()

    def attach(self, observer: IObserver) -> None:
        self.observers.append(observer)

    def detach(self, observer: IObserver) -> None:
        self.observers.remove(observer)

    def notify(self) -> None:
        for observer in self.observers:
            observer.update()

    def select_category(self, category_name: str) -> None:
        """
        Выбрать категорию и начать игру
        :param category_name: название категории
        """
        if category_name in self.categories:
            self.state.selected_category = self.categories[category_name]
            self.state.in_menu = False
            self.state.in_game = True
            self.state.word = self.state.selected_category.get_random_word()
            self.state.word_split = list(self.state.word)
            self.state.start_time = timer()
            self.notify()

    def guess_letter(self, letter: str) -> None:
        """
        Обработать угадывание буквы
        :param letter: буква
        """
        if self.state.in_game and not self.state.in_pause:
            self.state.check_guess(letter)
            if self.state.is_win():
                self.state.end_time = timer()
                self.state.win = True
                self.state.in_game = False
                self.state.in_end = True
            elif self.state.is_lose():
                self.state.end_time = timer()
                self.state.win = False
                self.state.in_game = False
                self.state.in_end = True
            self.notify()

    def pause(self) -> None:
        """
        Приостановить игру
        """
        if self.state.in_game and not self.state.in_pause:
            self.state.pause_start = timer()
            self.state.in_pause = True
            self.notify()

    def unpause(self) -> None:
        """
        Возобновить игру
        """
        if self.state.in_pause:
            pause_duration = timer() - self.state.pause_start
            self.state.start_time += pause_duration
            self.state.in_pause = False
            self.notify()

    def back_to_menu(self) -> None:
        """
        Вернуться в меню
        """
        self.state.reset()
        self.notify()

    def end_game(self) -> None:
        """
        Завершить приложение
        """
        sys.exit()

class View(IObserver):
    """
    Класс View, отвечающий за отрисовку
    """
    DEFAULT_WIDTH: int = 800
    DEFAULT_HEIGHT: int = 600
    BLACK: Tuple[int, int, int] = (0, 0, 0)
    WHITE: Tuple[int, int, int] = (255, 255, 255)
    LIGHTRED: Tuple[int, int, int] = (255, 165, 145)
    DARKLIGHTRED: Tuple[int, int, int] = (255, 97, 81)
    LIGHTBLUE: Tuple[int, int, int] = (126, 178, 255)
    DARKLIGHTBLUE: Tuple[int, int, int] = (42, 129, 255)
    LIGHTGREY: Tuple[int, int, int] = (192, 192, 192)

    def __init__(self, view: IView, game: HangmanGame) -> None:
        self.view = view
        self.game = game

    def update(self) -> None:
        """
        Обновить вид на основе состояния игры
        """
        self.view.fill_screen(self.WHITE)
        if self.game.state.in_menu:
            self.render_menu()
        elif self.game.state.in_pause:
            self.render_pause()
        elif self.game.state.in_game:
            self.render_game()
        elif self.game.state.in_end:
            self.render_end()
        self.view.update_display()

    def draw_button(self, text: str, x: int, y: int, w: int, h: int, inactive_color: Tuple[int, int, int], active_color: Tuple[int, int, int]) -> None:
        """
        Нарисовать кнопку
        :param text: Текст
        :param x: координата x верхнего левого угла
        :param y: координата y верхнего левого угла
        :param w: ширина
        :param h: высота
        :param inactive_color: обычный цвет
        :param active_color: цвет при наведении курсора
        :return:
        """
        mouse = self.view.get_mouse_pos()
        if x + w > mouse[0] > x and y + h > mouse[1] > y:
            self.view.draw_rect(active_color, (x, y, w, h))
        else:
            self.view.draw_rect(inactive_color, (x, y, w, h))
        self.view.draw_text(text, 20, (255, 255, 255), (x + w // 2, y + h // 2))

    def render_menu(self) -> None:
        """
        Отрисовать меню
        """
        self.view.draw_text("Choose a category", 20, self.BLACK, (self.DEFAULT_WIDTH // 2, self.DEFAULT_HEIGHT // 2))
        self.draw_button("Animals", 150, 450, 150, 100, self.BLACK, self.LIGHTGREY)
        self.draw_button("Vehicles", 550, 450, 150, 100, self.BLACK, self.LIGHTGREY)
        self.draw_button("Foods", 150, 50, 150, 100, self.BLACK, self.LIGHTGREY)
        self.draw_button("Sports", 550, 50, 150, 100, self.BLACK, self.LIGHTGREY)

    def render_game(self) -> None:
        """
        Отрисовать игру
        """
        if self.game.state.selected_category:
            title = self.game.state.selected_category.get_title()
            self.view.draw_text(title, 40, self.BLACK, (self.DEFAULT_WIDTH // 2, 50))

        space = 10
        for _ in range(len(self.game.state.word)):
            self.view.draw_text("_", 40, self.BLACK, (150 + space, 200))
            space += 60

        space = 10
        for i, char in enumerate(self.game.state.word_split):
            if char in self.game.state.guessed_letters:
                self.view.draw_text(char, 40, self.BLACK, (150 + space, 200))
            space += 60

        self.view.draw_rect(self.WHITE, (550, 20, 200, 20))
        self.view.draw_text(f"Chances: {self.game.state.chances}", 20, self.BLACK, (650, 20), center=False)

        self.view.draw_rect(self.BLACK, (100, 300, 250, 250), 2)
        text_box_space = 5
        text_box_number = 0
        for letter in self.game.state.wrong_letters:
            if text_box_number < 5:
                y_pos = 350
            elif text_box_number < 10:
                y_pos = 400
                if text_box_number == 5:
                    text_box_space = 5
            elif text_box_number < 15:
                y_pos = 450
                if text_box_number == 10:
                    text_box_space = 5
            else:
                y_pos = 500
                if text_box_number == 15:
                    text_box_space = 5
            text_box_space += 40
            text_box_number += 1
            self.view.draw_text(letter, 40, self.BLACK, (100 + text_box_space, y_pos))

        self.game.hangman_figure.draw(self.view, 19 - self.game.state.chances)

        self.draw_button("Back", 50, 50, 100, 50, self.BLACK, self.LIGHTGREY)

    def render_pause(self) -> None:
        """
        Отрисовать паузу
        """
        self.view.draw_text("Paused", 115, self.BLACK, (self.DEFAULT_WIDTH // 2, self.DEFAULT_HEIGHT // 2), center=True)
        self.draw_button("Continue", 150, 450, 100, 50, self.DARKLIGHTRED, self.LIGHTRED)
        self.draw_button("Quit", 550, 450, 100, 50, self.DARKLIGHTBLUE, self.LIGHTBLUE)

    def render_end(self) -> None:
        """
        Отрисовать конец игры
        """
        self.render_game()
        time_taken = round(self.game.state.get_time_taken())
        message = f"Time taken: {time_taken}s"

        if self.game.state.win:
            self.view.draw_text("You win!", 100, self.DARKLIGHTRED, (self.DEFAULT_WIDTH // 2, self.DEFAULT_HEIGHT // 2 - 200))
        else:
            self.view.draw_text("You lose!", 100, self.DARKLIGHTRED, (self.DEFAULT_WIDTH // 2, self.DEFAULT_HEIGHT // 2 - 200))

        self.view.draw_text("End Game?", 100, self.DARKLIGHTRED, (self.DEFAULT_WIDTH // 2, self.DEFAULT_HEIGHT // 2))
        self.view.draw_text(message, 70, self.DARKLIGHTRED, (self.DEFAULT_WIDTH // 2, self.DEFAULT_HEIGHT // 2 + 100))
        self.draw_button("Yes", (self.DEFAULT_WIDTH // 2) - 150, 450, 100, 50, self.DARKLIGHTRED, self.LIGHTRED)
        self.draw_button("No", (self.DEFAULT_WIDTH // 2) + 50, 450, 100, 50, self.DARKLIGHTRED, self.LIGHTRED)

class User:
    """
    Класс для обработки пользовательского ввода (Controller)
    """
    def __init__(self, user: IUser, game: HangmanGame) -> None:
        self.user = user
        self.game = game

    def handle_input(self) -> None:
        """
        Обработать ввод пользователя - нажатие клавиш или мыши
        """
        keys = self.user.get_key_events()
        mouse_pos = self.user.get_mouse_pos()
        mouse_click = self.user.get_mouse_click()

        for key in keys:
            if key == 'escape':
                self.game.end_game()
            elif key == 'space':
                if self.game.state.in_game and not self.game.state.in_pause:
                    self.game.pause()
            elif len(key) == 1 and 'a' <= key <= 'z':
                if self.game.state.in_game and not self.game.state.in_pause:
                    self.game.guess_letter(key)

        if mouse_click[0]:
            if self.game.state.in_menu:
                if 150 < mouse_pos[0] < 300 and 450 < mouse_pos[1] < 550:
                    self.game.select_category("Animals")
                elif 550 < mouse_pos[0] < 700 and 450 < mouse_pos[1] < 550:
                    self.game.select_category("Vehicles")
                elif 150 < mouse_pos[0] < 300 and 50 < mouse_pos[1] < 150:
                    self.game.select_category("Foods")
                elif 550 < mouse_pos[0] < 700 and 50 < mouse_pos[1] < 150:
                    self.game.select_category("Sports")
            elif self.game.state.in_game and not self.game.state.in_pause:
                if 50 < mouse_pos[0] < 150 and 50 < mouse_pos[1] < 100:
                    self.game.back_to_menu()
            elif self.game.state.in_pause:
                if 150 < mouse_pos[0] < 250 and 450 < mouse_pos[1] < 500:
                    self.game.unpause()
                elif 550 < mouse_pos[0] < 650 and 450 < mouse_pos[1] < 500:
                    self.game.end_game()
            elif self.game.state.in_end:
                if 250 < mouse_pos[0] < 350 and 450 < mouse_pos[1] < 500:
                    self.game.end_game()
                elif 450 < mouse_pos[0] < 550 and 450 < mouse_pos[1] < 500:
                    self.game.back_to_menu()

def main() -> None:
    renderer = PygameRenderer()
    game = HangmanGame()
    view = View(renderer, game)
    user = User(renderer, game)
    game.attach(view)
    renderer.init_screen("Hangman!")
    while True:
        user.handle_input()
        view.update()
        renderer.tick(30)

if __name__ == "__main__":
    main()
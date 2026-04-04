import curses
from abc import ABC, abstractmethod

class IControllerAdapter(ABC):

    @abstractmethod
    def get_key(self):
        pass

    @abstractmethod
    def get_window_size(self):
        pass

class IViewAdapter(ABC):

    @abstractmethod
    def addstr(self, y: int, x: int, text: str, attr: int = 0):
        pass

    @abstractmethod
    def move(self, y: int, x: int):
        pass

    @abstractmethod
    def refresh(self):
        pass

    @abstractmethod
    def draw_status_bar(self, text: str):
        pass

    @abstractmethod
    def clear(self):
        pass

    @abstractmethod
    def get_window_size(self):
        pass

    @abstractmethod
    def init_colors(self):
        pass

    @abstractmethod
    def color_pair(self, pair_number: int) -> int:
        pass

    @abstractmethod
    def has_colors(self) -> bool:
        pass

class CursesAdapter(IControllerAdapter, IViewAdapter):

    ENTER = curses.KEY_ENTER
    BACKSPACE = curses.KEY_BACKSPACE
    LEFT = curses.KEY_LEFT
    RIGHT = curses.KEY_RIGHT
    UP = curses.KEY_UP
    DOWN = curses.KEY_DOWN
    PPAGE = curses.KEY_PPAGE
    NPAGE = curses.KEY_NPAGE

    COLOR_KEYWORD = 1
    COLOR_STRING = 2
    COLOR_COMMENT = 3
    COLOR_NUMBER = 4
    COLOR_FUNCTION = 5
    COLOR_MAGIC = 6

    def __init__(self, stdscr):
        self.stdscr = stdscr
        curses.curs_set(1)
        self.stdscr.keypad(True)
        self.stdscr.nodelay(False)
        self.init_colors()

    def init_colors(self):
        if curses.has_colors():
            curses.start_color()
            curses.use_default_colors()
            # curses.init_pair(pair_number, foreground, background)
            curses.init_pair(self.COLOR_KEYWORD, curses.COLOR_BLUE, curses.COLOR_BLACK)
            curses.init_pair(self.COLOR_STRING, curses.COLOR_GREEN, curses.COLOR_BLACK)
            curses.init_pair(self.COLOR_COMMENT, curses.COLOR_CYAN, curses.COLOR_BLACK)
            curses.init_pair(self.COLOR_NUMBER, curses.COLOR_RED, curses.COLOR_BLACK)
            curses.init_pair(self.COLOR_FUNCTION, curses.COLOR_YELLOW, curses.COLOR_BLACK)
            curses.init_pair(self.COLOR_MAGIC, curses.COLOR_MAGENTA, curses.COLOR_BLACK)

    def color_pair(self, pair_number: int) -> int:
        return curses.color_pair(pair_number)

    def has_colors(self) -> bool:
        return curses.has_colors()

    def get_key(self):
        return self.stdscr.getch()

    def get_cursor_pos(self):
        return self.stdscr.getyx()

    def clear(self):
        self.stdscr.clear()

    def get_window_size(self):
        return self.stdscr.getmaxyx()

    def refresh(self):
        self.stdscr.refresh()

    def move(self, y, x):
        self.stdscr.move(y, x)

    def addstr(self, y, x, text, attr=0):
        try:
            self.stdscr.addstr(y, x, text, attr)
        except curses.error:
            pass

    def draw_status_bar(self, text):
        h, w = self.stdscr.getmaxyx()
        if h <= 1:
            return
        status = text.ljust(w)[:w]
        self.stdscr.attron(curses.A_REVERSE)
        try:
            self.stdscr.addstr(h - 1, 0, status)
        except curses.error:
            pass
        self.stdscr.attroff(curses.A_REVERSE)

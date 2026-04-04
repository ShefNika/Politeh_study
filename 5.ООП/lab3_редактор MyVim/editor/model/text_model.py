from typing import List, Callable
from MyString import MyString
from mvc_framework.base import Model
from editor.controller.history import CommandHistory


class EditorModel(Model):

    def __init__(self):
        super().__init__()
        self.lines: List[MyString] = [MyString("")]
        self.cursor_x = 0
        self.cursor_y = 0
        self.filename = None
        self.modified = False
        self.mode = "NORMAL"
        self.command_buffer = MyString()
        self.clipboard: List[MyString] = []
        self.initial_line_content = {}
        self.search_pattern = MyString()
        self.search_direction = 1
        self.scroll_offset = 0
        self.last_error = MyString()
        self.help_text: List[MyString] = []
        self.initial_file_state = None
        self.show_line_numbers = False
        self.syntax_highlight = False
        self.line_heights = [1]
        self.total_virtual_lines = 1
        self.screen_width = 0
        self.screen_height = 0
        self._saved_state_before_help = None

    def ensure_cursor_visible(self, screen_height: int):
        if self.cursor_y < self.scroll_offset:
            self.scroll_offset = self.cursor_y
            return
        visible = 0
        y = self.scroll_offset
        while y < len(self.lines) and visible < screen_height - 1:
            if y == self.cursor_y:
                return
            visible += self.line_heights[y]
            y += 1
        self.scroll_offset = self.cursor_y

    def set_mode(self, mode: str):
        self.mode = mode
        self.notify()

    def save_initial_line_state(self):
        current_line = self.cursor_y
        if current_line not in self.initial_line_content:
            self.initial_line_content[current_line] = MyString(self.lines[current_line])

    def undo_last_command(self):
        history = CommandHistory()
        if history.commands:
            last_command, previous_state = history.get_last_command()
            if previous_state:
                self.restore_state(previous_state)
        else:
            self.set_error('No commands before')

    def get_state(self):
        lines_copy = []
        for line in self.lines:
            lines_copy.append(MyString(line))
        return {
            'lines': lines_copy,
            'cursor_x': self.cursor_x,
            'cursor_y': self.cursor_y,
            'modified': self.modified
        }

    def restore_state(self, state):
        if state:
            self.lines = state['lines']
            self.cursor_x = state['cursor_x']
            self.cursor_y = state['cursor_y']
            self.modified = state['modified']
            self.notify()

    def undo_line_changes(self):
        current_line = self.cursor_y
        if current_line in self.initial_line_content:
            original_content = self.initial_line_content[current_line]
            self.lines[current_line] = MyString(original_content)
            self.cursor_x = self.lines[current_line].size()
            self.modified = True
            self.notify()
        if current_line in self.initial_line_content:
            del self.initial_line_content[current_line]

    def move_cursor(self, dx=0, dy=0):
        self.cursor_y = max(0, min(self.cursor_y + dy, len(self.lines) - 1))
        line_len = self.lines[self.cursor_y].size()
        self.cursor_x = max(0, min(self.cursor_x + dx, line_len))
        self.ensure_cursor_visible(self.screen_height)
        self.notify()

    def _recalc_line_height(self, line_idx: int, text_width: int):
        while len(self.line_heights) < len(self.lines):
            self.line_heights.append(1)
            self.total_virtual_lines += 1
        line_len = self.lines[line_idx].size()
        old = self.line_heights[line_idx]
        new = max(1, (line_len + text_width - 1) // text_width)
        self.line_heights[line_idx] = new
        self.total_virtual_lines += (new - old)

    def recalc_all_line_heights(self, text_width: int):
        self.line_heights = []
        self.total_virtual_lines = 0

        for line in self.lines:
            h = max(1, (line.size() + text_width - 1) // text_width)
            self.line_heights.append(h)
            self.total_virtual_lines += h

        if not self.line_heights:
            self.line_heights = [1]
            self.total_virtual_lines = 1

    def insert_char(self, ch: str):
        self.lines[self.cursor_y].insert(self.cursor_x, 1, ch)
        self.cursor_x += 1
        self.modified = True
        self._recalc_line_height(self.cursor_y, self.screen_width)
        self.notify()

    def replace_char(self, ch: str):
        if (self.cursor_x < self.lines[self.cursor_y].size() and
                self.lines[self.cursor_y].size() > 0):
            self.lines[self.cursor_y].erase(self.cursor_x, 1)
            self.lines[self.cursor_y].insert(self.cursor_x, 1, ch)
            self.modified = True
            self.notify()

    def new_line_below(self):
        current_line = self.lines[self.cursor_y]
        if self.cursor_x < current_line.size():
            rest = current_line.substr(self.cursor_x)
            current_line.erase(self.cursor_x, current_line.size() - self.cursor_x)
        else:
            rest = MyString("")
        self.lines.insert(self.cursor_y + 1, rest)
        self.line_heights.insert(self.cursor_y + 1, 1)
        self.total_virtual_lines += 1
        self.cursor_y += 1
        self.cursor_x = 0
        self.modified = True
        self._recalc_line_height(self.cursor_y, self.screen_width)
        self.notify()

    def clear_line_and_insert(self):
        self.lines[self.cursor_y].clear()
        self.cursor_x = 0
        self.modified = True
        self._recalc_line_height(self.cursor_y, self.screen_width)
        self.notify()

    def backspace(self):
        if self.cursor_x > 0:
            self.lines[self.cursor_y].erase(self.cursor_x - 1, 1)
            self.cursor_x -= 1
            self.modified = True
            self._recalc_line_height(self.cursor_y, self.screen_width)
            self.notify()
        elif self.cursor_y > 0:
            prev_len = self.lines[self.cursor_y - 1].size()
            self.lines[self.cursor_y - 1].append(self.lines[self.cursor_y])
            del self.lines[self.cursor_y]
            self.cursor_y -= 1
            self.cursor_x = prev_len
            self.modified = True
            self._recalc_line_height(self.cursor_y, self.screen_width)
            self.notify()

    def move_to_next_word(self):
        current_line = self.lines[self.cursor_y]
        current_pos = self.cursor_x
        if current_pos >= current_line.size():
            if self.cursor_y < len(self.lines) - 1:
                self.cursor_y += 1
                self.cursor_x = 0
            self.notify()
            return
        pos = current_pos
        while pos < current_line.size() and current_line[pos] == ' ':
            pos += 1
        while pos < current_line.size() and current_line[pos] != ' ':
            pos += 1
        if pos < current_line.size():
            word_end = pos
            while word_end < current_line.size() and current_line[word_end] != ' ':
                word_end += 1
            self.cursor_x = word_end
        else:
            self.cursor_x = current_line.size()
        self.notify()

    def move_to_prev_word(self):
        current_line = self.lines[self.cursor_y]
        current_pos = self.cursor_x
        if current_pos == 0:
            if self.cursor_y > 0:
                self.cursor_y -= 1
                self.cursor_x = self.lines[self.cursor_y].size()
            self.notify()
            return
        pos = current_pos - 1
        while pos >= 0 and current_line[pos] == ' ':
            pos -= 1
        while pos >= 0 and current_line[pos] != ' ':
            pos -= 1
        self.cursor_x = pos + 1
        self.notify()

    def move_to_file_start(self):
        self.cursor_y = 0
        self.cursor_x = 0
        self.ensure_cursor_visible(self.screen_height)
        self.notify()

    def move_to_file_end(self):
        self.cursor_y = len(self.lines) - 1
        self.cursor_x = self.lines[self.cursor_y].size()
        self.ensure_cursor_visible(self.screen_height)
        self.notify()

    def delete_char(self):
        if self.cursor_x < self.lines[self.cursor_y].size():
            self.lines[self.cursor_y].erase(self.cursor_x, 1)
            self.modified = True
            self._recalc_line_height(self.cursor_y, self.screen_width)
            self.notify()

    def delete_word(self):
        current_line = self.lines[self.cursor_y]
        if self.cursor_x >= current_line.size():
            return

        start = self.cursor_x
        if current_line[start] == ' ':
            while start > 0 and current_line[start] == ' ':
                start -= 1
            while start > 0 and current_line[start - 1] != ' ':
                start -= 1
            end = start
            while end < current_line.size() and current_line[end] != ' ':
                end += 1
            while end < current_line.size() and current_line[end] == ' ':
                end += 1
        else:
            while start > 0 and current_line[start - 1] != ' ':
                start -= 1
            end = self.cursor_x
            while end < current_line.size() and current_line[end] != ' ':
                end += 1
            while end < current_line.size() and current_line[end] == ' ':
                end += 1
        if start < end:
            current_line.erase(start, end - start)
            self.cursor_x = start
            self.modified = True
            self.notify()

    def cut_line(self):
        self.clipboard = [MyString(self.lines[self.cursor_y])]
        removed_height = self.line_heights[self.cursor_y]
        del self.lines[self.cursor_y]
        del self.line_heights[self.cursor_y]
        self.total_virtual_lines -= removed_height
        if not self.lines:
            self.lines.append(MyString(""))
        self.cursor_y = min(self.cursor_y, len(self.lines) - 1)
        self.cursor_x = min(self.cursor_x, self.lines[self.cursor_y].size())
        self.modified = True
        self._recalc_line_height(self.cursor_y, self.screen_width)
        self.notify()

    def copy_line(self):
        self.clipboard = [MyString(self.lines[self.cursor_y])]
        self.notify()

    def copy_word(self):
        current_line = self.lines[self.cursor_y]
        if (self.cursor_x >= current_line.size() or
                current_line[self.cursor_x] == ' '):
            return

        start = self.cursor_x
        end = self.cursor_x
        while start > 0 and current_line[start - 1] != ' ':
            start -= 1
        while end < current_line.size() and current_line[end] != ' ':
            end += 1

        word = current_line.substr(start, end - start)
        self.clipboard = [word]
        self.notify()

    def paste_after(self):
        if not self.clipboard:
            return
        for line in self.clipboard:
            if line.size() > 0:
                self.lines[self.cursor_y].insert(self.cursor_x, line)
                self.cursor_x += line.size()
        self.modified = True
        self._recalc_line_height(self.cursor_y, self.screen_width)
        self.notify()

    def move_to_line_number(self, line_num: int):
        target_line = line_num - 1
        if target_line < 0:
            target_line = 0
        elif target_line >= len(self.lines):
            target_line = len(self.lines) - 1
        self.cursor_y = target_line
        self.cursor_x = 0
        self.ensure_cursor_visible(self.screen_height)
        self.notify()

    def page_down(self, screen_height: int):
        remaining = screen_height - 1
        y = self.scroll_offset
        while y < len(self.lines) and remaining > 0:
            remaining -= self.line_heights[y]
            y += 1
        if y >= len(self.lines):
            y = len(self.lines) - 1
        self.scroll_offset = y
        self.cursor_y = y
        self.cursor_x = min(self.cursor_x, self.lines[y].size())
        self.notify()

    def page_up(self, screen_height: int):
        remaining = screen_height - 1
        y = self.scroll_offset - 1
        while y >= 0 and remaining > 0:
            remaining -= self.line_heights[y]
            y -= 1
        self.scroll_offset = max(0, y + 1)
        self.cursor_y = self.scroll_offset
        self.cursor_x = min(self.cursor_x, self.lines[self.cursor_y].size())
        self.notify()

    def start_search(self, direction: int):
        self.search_direction = direction
        self.search_pattern.clear()
        self.set_mode("SEARCH")
        self.notify()

    def execute_search(self):
        if not self.search_pattern:
            self.set_mode("NORMAL")
            return
        if self.search_direction == 1:
            if not self._search_forward():
                self.set_error('Not found')
        else:
            if not self._search_backward():
                self.set_error('Not found')
        self.set_mode("NORMAL")
        self.ensure_cursor_visible(self.screen_height)
        self.notify()

    def _search_forward(self):
        start_line = self.cursor_y
        start_pos = self.cursor_x + 1
        if start_pos < self.lines[start_line].size():
            found_pos = self.lines[start_line].find(self.search_pattern, start_pos)
            if found_pos != -1:
                self.cursor_x = found_pos
                return True
        for line_idx in range(start_line + 1, len(self.lines)):
            line = self.lines[line_idx]
            if line.size() == 0:
                continue
            found_pos = line.find(self.search_pattern, 0)
            if found_pos != -1:
                self.cursor_y = line_idx
                self.cursor_x = found_pos
                return True
        for line_idx in range(0, start_line + 1):
            line = self.lines[line_idx]
            if line.size() == 0:
                continue
            if line_idx == start_line:
                found_pos = line.find(self.search_pattern, 0)
                if found_pos != -1 and found_pos < self.cursor_x:
                    self.cursor_x = found_pos
                    return True
            else:
                found_pos = line.find(self.search_pattern, 0)
                if found_pos != -1:
                    self.cursor_y = line_idx
                    self.cursor_x = found_pos
                    return True
        return False

    def _search_backward(self):
        start_line = self.cursor_y
        start_pos = max(0, self.cursor_x - 1)
        for pos in range(start_pos, -1, -1):
            if pos + len(self.search_pattern) <= self.lines[start_line].size():
                substr = self.lines[start_line].substr(pos, len(self.search_pattern))
                if substr == self.search_pattern:
                    self.cursor_x = pos
                    return True
        for line_idx in range(start_line - 1, -1, -1):
            line = self.lines[line_idx]
            if line.size() == 0:
                continue
            for pos in range(line.size() - len(self.search_pattern), -1, -1):
                substr = line.substr(pos, len(self.search_pattern))
                if substr == self.search_pattern:
                    self.cursor_y = line_idx
                    self.cursor_x = pos
                    return True
        for line_idx in range(len(self.lines) - 1, start_line - 1, -1):
            line = self.lines[line_idx]
            if line.size() == 0:
                continue
            if line_idx == start_line:
                for pos in range(line.size() - len(self.search_pattern), self.cursor_x, -1):
                    substr = line.substr(pos, len(self.search_pattern))
                    if substr == self.search_pattern:
                        self.cursor_x = pos
                        return True
            else:
                for pos in range(line.size() - len(self.search_pattern), -1, -1):
                    substr = line.substr(pos, len(self.search_pattern))
                    if substr == self.search_pattern:
                        self.cursor_y = line_idx
                        self.cursor_x = pos
                        return True
            return False

    def repeat_search_reverse(self):
        if not self.search_pattern:
            return
        original_direction = self.search_direction
        self.search_direction = -self.search_direction
        if self.search_direction == 1:
            if not self._search_forward():
                self.set_error('Not found')
        else:
            if not self._search_backward():
                self.set_error('Not found')
        self.search_direction = original_direction
        self.set_mode("NORMAL")
        self.ensure_cursor_visible(self.screen_height)
        self.notify()

    def open_file(self, filename: str):
        try:
            with open(filename, 'r', encoding='latin-1') as f:
                new_lines = []
                for line in f:
                    line = line.rstrip('\n\r')
                    new_lines.append(MyString(line))
                if not new_lines:
                    new_lines.append(MyString(""))
                self.lines = new_lines
                self.recalc_all_line_heights(self.screen_width)
                self.filename = filename
                self.modified = False
                self.cursor_x = 0
                self.cursor_y = 0
                self.initial_file_state = self.get_state()
                history = CommandHistory()
                history.commands.clear()
                history.states.clear()
                self.initial_line_content.clear()
                filename_only = self.filename.split('\\')[-1]
                self.set_error(f"Opened file: {filename_only}")
        except FileNotFoundError:
            self.lines = [MyString("")]
            self.recalc_all_line_heights(self.screen_width)
            self.filename = filename
            self.modified = False
            self.set_error(f"New file: {filename}")
        except Exception as e:
            self.set_error(f"Error: {str(e)}")
        self.notify()

    def save_file(self):
        if not self.filename:
            self.set_error("No file name")
            self.notify()
            return False
        try:
            with open(self.filename, 'w', encoding='latin-1') as f:
                for line in self.lines:
                    f.write(line.c_str() + '\n')
            self.modified = False
            self.set_error(f"Saved: {self.filename}")
            self.notify()
            return True
        except Exception as e:
            self.set_error(f"Error: {str(e)}")
            self.notify()
            return False

    def save_file_as(self, filename: str):
        old_filename = self.filename
        self.filename = filename
        if self.save_file():
            self.set_error(f"Saved as: {self.filename}")
        else:
            self.filename = old_filename
        self.notify()

    def show_help(self):
        self._saved_state_before_help = self.get_state()
        try:
            with open("help.txt", "r", encoding="latin-1") as f:
                self.lines = [MyString(line.rstrip('\n\r')) for line in f]
                if not self.lines:
                    self.lines = [MyString("")]
        except Exception as e:
            self.lines = [MyString("Failed to load help.txt")]
            self.last_error = MyString(str(e))
        self.cursor_x = 0
        self.cursor_y = 0
        self.scroll_offset = 0
        self.modified = False
        self.mode = "HELP"
        self.notify()

    def add_to_command_buffer(self, char: str):
        self.command_buffer.append(char)
        self.notify()

    def remove_from_command_buffer(self):
        if self.command_buffer.size() > 0:
            self.command_buffer.erase(self.command_buffer.size() - 1, 1)
            self.notify()

    def add_to_search_pattern(self, char: str):
        self.search_pattern.append(char)
        self.notify()

    def remove_from_search_pattern(self):
        if self.search_pattern.size() > 0:
            self.search_pattern.erase(self.search_pattern.size() - 1, 1)
            self.notify()

    def set_error(self, error_msg: str):
        self.last_error = MyString(error_msg)
        self.notify()

    def undo_all_changes(self):
        history = CommandHistory()
        if history.states:
            first_state = history.states[0]
            self.restore_state(first_state)
            history.commands.clear()
            history.states.clear()
        elif self.initial_file_state:
            self.restore_state(self.initial_file_state)
        else:
            self.set_error("No changes to undo")
            return
        self.initial_line_content.clear()
        self.modified = False
        self.set_error("All changes undone")
        self.notify()

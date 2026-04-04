from abc import ABC, abstractmethod
from editor.controller.history import CommandHistory


class CommandInvoker:
    @staticmethod
    def execute(command):
        history = CommandHistory()

        model_state = None
        if hasattr(command, 'ctrl'):
            model = command.ctrl.model
            model_state = model.get_state()
            editing_commands = [
                'EnterInsertMode', 'EnterInsertAfter', 'ReplaceChar', 'ClearLineAndInsert',
                'DeleteCharAfter', 'DeleteWord', 'CutLine', 'PasteAfter', 'OpenFileCommand'
            ]
            if command.__class__.__name__ in editing_commands:
                command.ctrl.model.save_initial_line_state()
        history.add(command, model_state)
        command.execute()


class Command(ABC):

    def __init__(self, controller):
        self.ctrl = controller

    @abstractmethod
    def execute(self): pass


class MoveRight(Command):
    def execute(self): self.ctrl.model.move_cursor(dx=1)


class MoveLeft(Command):
    def execute(self): self.ctrl.model.move_cursor(dx=-1)


class MoveDown(Command):
    def execute(self): self.ctrl.model.move_cursor(dy=1)


class MoveUp(Command):
    def execute(self): self.ctrl.model.move_cursor(dy=-1)


class MoveToLineStart(Command):
    def execute(self):
        self.ctrl.model.cursor_x = 0
        self.ctrl.model.notify()


class MoveToLineEnd(Command):
    def execute(self):
        self.ctrl.model.cursor_x = self.ctrl.model.lines[self.ctrl.model.cursor_y].size()
        self.ctrl.model.notify()


class EnterInsertMode(Command):
    def execute(self): self.ctrl.model.set_mode("INSERT")


class EnterInsertAfter(Command):
    def execute(self):
        ctrl = self.ctrl
        if ctrl.model.cursor_x < ctrl.model.lines[ctrl.model.cursor_y].size():
            ctrl.model.cursor_x += 1
        ctrl.model.set_mode("INSERT")


class ClearLineAndInsert(Command):
    def execute(self):
        self.ctrl.model.clear_line_and_insert()
        self.ctrl.model.set_mode("INSERT")


class EnterInsertAtLineStart(Command):
    def execute(self):
        self.ctrl.model.cursor_x = 0
        self.ctrl.model.set_mode("INSERT")
        self.ctrl.model.notify()


class EnterInsertAtLineEnd(Command):
    def execute(self):
        self.ctrl.model.cursor_x = self.ctrl.model.lines[self.ctrl.model.cursor_y].size()
        self.ctrl.model.set_mode("INSERT")
        self.ctrl.model.notify()


class ReplaceChar(Command):
    def execute(self):
        self.ctrl.model.set_mode("REPLACE")
        self.ctrl.model.notify()


class EnterCommandMode(Command):
    def execute(self): self.ctrl.model.set_mode("COMMAND")


class MoveToNextWord(Command):
    def execute(self):
        self.ctrl.model.move_to_next_word()


class MoveToPrevWord(Command):
    def execute(self):
        self.ctrl.model.move_to_prev_word()


class MoveToFileStart(Command):
    def execute(self):
        self.ctrl.model.move_to_file_start()


class MoveToFileEnd(Command):
    def execute(self):
        self.ctrl.model.move_to_file_end()


class DeleteCharAfter(Command):
    def execute(self):
        self.ctrl.model.delete_char()


class DeleteWord(Command):
    def execute(self):
        self.ctrl.model.delete_word()


class CutLine(Command):
    def execute(self):
        self.ctrl.model.cut_line()


class CopyLine(Command):
    def execute(self):
        self.ctrl.model.copy_line()


class CopyWord(Command):
    def execute(self):
        self.ctrl.model.copy_word()


class PasteAfter(Command):
    def execute(self):
        self.ctrl.model.paste_after()


class UndoCommand(Command):
    def execute(self):
        self.ctrl.model.undo_last_command()


class UndoLineChanges(Command):
    def execute(self):
        self.ctrl.model.undo_line_changes()

class UndoAllChanges(Command):
    def execute(self):
        self.ctrl.model.undo_all_changes()


class MoveToLineNumber(Command):
    def __init__(self, controller, line_num: int):
        super().__init__(controller)
        self.line_num = line_num

    def execute(self):
        self.ctrl.model.move_to_line_number(self.line_num)


class PageUp(Command):
    def __init__(self, controller, screen_height: int):
        super().__init__(controller)
        self.screen_height = screen_height
    def execute(self):
        self.ctrl.model.page_up(self.screen_height)


class PageDown(Command):
    def __init__(self, controller, screen_height: int):
        super().__init__(controller)
        self.screen_height = screen_height
    def execute(self):
        self.ctrl.model.page_down(self.screen_height)

class StartSearchForward(Command):
    def execute(self):
        self.ctrl.model.start_search(1)

class StartSearchBackward(Command):
    def execute(self):
        self.ctrl.model.start_search(-1)

class RepeatSearch(Command):
    def execute(self):
        self.ctrl.model.execute_search()

class RepeatSearchReverse(Command):
    def execute(self):
        self.ctrl.model.repeat_search_reverse()

class OpenFile(Command):
    def __init__(self, controller, filename: str):
        super().__init__(controller)
        self.filename = filename

    def execute(self):
        self.ctrl.model.open_file(self.filename)


class SaveFile(Command):
    def execute(self):
        self.ctrl.model.save_file()


class SaveFileAs(Command):
    def __init__(self, controller, filename: str):
        super().__init__(controller)
        self.filename = filename

    def execute(self):
        self.ctrl.model.save_file_as(self.filename)


class Quit(Command):
    def execute(self):
        if not self.ctrl.model.modified:
            self.ctrl.stop()
        else:
            self.ctrl.model.set_error('File has been changed!')


class ForceQuit(Command):
    def execute(self):
        self.ctrl.stop()


class SaveAndQuit(Command):
    def execute(self):
        self.ctrl.model.save_file()
        if not self.ctrl.model.modified:
            self.ctrl.stop()


class ShowHelp(Command):
    def execute(self):
        self.ctrl.model.show_help()


class SwitchLineNumbers(Command):
    def execute(self):
        self.ctrl.model.show_line_numbers = not self.ctrl.model.show_line_numbers
        self.ctrl.model.set_error(
            f"Line numbers {'ON' if self.ctrl.model.show_line_numbers else 'OFF'}"
        )

class SwitchSyntaxHighlight(Command):
    def execute(self):
        self.ctrl.model.syntax_highlight = not self.ctrl.model.syntax_highlight
        self.ctrl.model.set_error(
            f"Syntax highlighting {'ON' if self.ctrl.model.syntax_highlight else 'OFF'}"
        )
from MyString import MyString
from mvc_framework.base import Controller
from editor.controller.commands import *
from editor.model.text_model import EditorModel
from editor.view.curses_adapter import IControllerAdapter



class EditorController(Controller):
    def __init__(self, adapter: IControllerAdapter, model: EditorModel):
        super().__init__(adapter, model)
        self.numeric_buffer = MyString()
        self.wait_after_number = False
        self.screen_height = 0
        self.screen_width = 0



    def run(self):
        self.screen_height, self.screen_width = self.adapter.get_window_size()
        self.model.screen_height = self.screen_height
        line_num_width = 0
        if self.model.show_line_numbers:
            total_lines = len(self.model.lines)
            line_num_width = len(str(total_lines)) + 1
        text_width = self.screen_width - line_num_width
        self.model.screen_width = text_width
        if self.model.filename:
            CommandInvoker.execute(OpenFile(self, self.model.filename))

        self.model.notify()

        while self.running:
            key = self.adapter.get_key()
            if self.model.last_error.size() > 0:
                self.model.last_error.clear()
            self.handle_key(key)

    def handle_key(self, key):
        if self.model.mode == "NORMAL":
            self.handle_normal_mode(key)
        elif self.model.mode == "INSERT":
            self.handle_insert_mode(key)
        elif self.model.mode == "COMMAND":
            self.handle_command_mode(key)
        elif self.model.mode == "REPLACE":
            self.handle_replace_mode(key)
        elif self.model.mode == "SEARCH":
            self.handle_search_mode(key)
        elif self.model.mode == "HELP":
            self.handle_help_mode(key)

    def handle_help_mode(self, key):
        if key == 27:
            if self.model._saved_state_before_help:
                self.model.restore_state(self.model._saved_state_before_help)
                self.model._saved_state_before_help = None
            self.model.set_mode("NORMAL")
            self.model.notify()

    def handle_normal_mode(self, key):
        if ord('0') <= key <= ord('9'):
            self.numeric_buffer.append(chr(key))
            self.wait_after_number = True
            return
        if self.wait_after_number and key == ord('G'):
            if self.numeric_buffer:
                line_num = int(self.numeric_buffer.c_str())
                CommandInvoker.execute(MoveToLineNumber(self, line_num))
            self.numeric_buffer.clear()
            self.waiting_for_command = False
            return
        if self.wait_after_number:
            self.numeric_buffer.clear()
            self.wait_after_number = False
        mapping = {
            self.adapter.RIGHT: MoveRight(self),
            self.adapter.LEFT:  MoveLeft(self),
            self.adapter.DOWN:  MoveDown(self),
            self.adapter.UP:    MoveUp(self),
            self.adapter.PPAGE: PageUp(self, self.screen_height),
            self.adapter.NPAGE: PageDown(self, self.screen_height),
            ord('0'): MoveToLineStart(self),
            ord('^'): MoveToLineStart(self),
            ord('$'): MoveToLineEnd(self),
            ord('i'): EnterInsertMode(self),
            ord('o'): EnterInsertAfter(self),
            ord('S'): ClearLineAndInsert(self),
            ord(':'): EnterCommandMode(self),
            ord('I'): EnterInsertAtLineStart(self),
            ord('A'): EnterInsertAtLineEnd(self),
            ord('r'): ReplaceChar(self),
            ord('w'): MoveToNextWord(self),
            ord('b'): MoveToPrevWord(self),
            ord('x'): DeleteCharAfter(self),
            ord('G'): MoveToFileEnd(self),
            ord('d'): self.handle_d_command,
            ord('y'): self.handle_y_command,
            ord('p'): PasteAfter(self),
            ord('g'): self.handle_g_command,
            ord('u'): UndoCommand(self),
            ord('U'): UndoLineChanges(self),
            ord('/'): StartSearchForward(self),
            ord('?'): StartSearchBackward(self),
            ord('n'): RepeatSearch(self),
            ord('N'): RepeatSearchReverse(self),
        }
        cmd = mapping.get(key)
        if cmd:
            if callable(cmd) and cmd in [self.handle_d_command, self.handle_y_command, self.handle_g_command]:
                cmd()
            else:
                CommandInvoker.execute(cmd)

    def handle_g_command(self):
        key = self.adapter.get_key()
        if key == ord('g'):
            CommandInvoker.execute(MoveToFileStart(self))

    def handle_y_command(self):
        key = self.adapter.get_key()
        if key == ord('y'):
            CommandInvoker.execute(CopyLine(self))
        elif key == ord('w'):
            CommandInvoker.execute(CopyWord(self))

    def handle_d_command(self):
        key = self.adapter.get_key()
        if key == ord('d'):
            CommandInvoker.execute(CutLine(self))
        elif key == ord('i'):
            key2 = self.adapter.get_key()
            if key2 == ord('w'):
                CommandInvoker.execute(DeleteWord(self))

    def handle_replace_mode(self, key):
        if key == 27:
            self.model.set_mode("NORMAL")
        elif 32 <= key <= 126:
            self.model.replace_char(chr(key))
            self.model.set_mode("NORMAL")
        else:
            self.model.set_mode("NORMAL")


    def handle_insert_mode(self, key):
        if key == 27:
            self.model.set_mode("NORMAL")
        elif key in (self.adapter.BACKSPACE, 127, 8):
            self.model.backspace()
        elif key == self.adapter.ENTER or key == 10:
            self.model.new_line_below()
        elif 32 <= key <= 126:
            self.model.insert_char(chr(key))

    def handle_search_mode(self, key):
        if key == 27:
            self.model.set_mode("NORMAL")
        elif key == self.adapter.ENTER or key == 10:
            self.model.execute_search()
        elif key in (self.adapter.BACKSPACE, 127, 8):
            if self.model.search_pattern.size() > 0:
                self.model.remove_from_search_pattern()
        elif 32 <= key <= 126:
            self.model.add_to_search_pattern(chr(key))

    def handle_command_mode(self, key):
        if key == 27:
            self.model.set_mode("NORMAL")
            self.model.command_buffer.clear()
            self.model.last_error.clear()
        elif key == self.adapter.ENTER or key == 10:
            cmd = self.model.command_buffer.c_str() if self.model.command_buffer.size() > 0 else ""
            self.model.command_buffer.clear()
            self.execute_command(cmd)
            if self.model.mode == "COMMAND":
                self.model.set_mode("NORMAL")
        elif key in (self.adapter.BACKSPACE, 127, 8):
            if self.model.command_buffer.size() > 0:
                self.model.remove_from_command_buffer()
        elif 32 <= key <= 126:
            self.model.add_to_command_buffer(chr(key))


    def execute_command(self, cmd: str):
        cmd = cmd.strip()
        if not cmd:
            return
        parts = cmd.split()
        cmd_name = parts[0]
        args = parts[1:] if len(parts) > 1 else []
        if cmd_name == "o" and args:
            CommandInvoker.execute(OpenFile(self, args[0]))
        elif cmd_name == "x":
            CommandInvoker.execute(SaveAndQuit(self))
        elif cmd_name == "w" and not args:
            CommandInvoker.execute(SaveFile(self))
        elif cmd_name == "w" and args:
            CommandInvoker.execute(SaveFileAs(self, args[0]))
        elif cmd_name == "q":
            CommandInvoker.execute(Quit(self))
        elif cmd_name == "e!":
            CommandInvoker.execute(UndoAllChanges(self))
        elif cmd_name == "q!":
            CommandInvoker.execute(ForceQuit(self))
        elif cmd_name == "wq!":
            CommandInvoker.execute(SaveAndQuit(self))
        elif cmd_name.isdigit():
            line_num = int(cmd_name)
            CommandInvoker.execute(MoveToLineNumber(self, line_num))
        elif cmd_name == "h":
            CommandInvoker.execute(ShowHelp(self))
        elif cmd_name == "set" and len(args) >= 1:
            CommandInvoker.execute(SwitchLineNumbers(self))
        elif cmd_name == "sy":
            CommandInvoker.execute(SwitchSyntaxHighlight(self))
        else:
            self.model.set_error(f"Unknown command: {cmd_name}")

    def stop(self):
        self.running = False
from mvc_framework.base import View
from editor.model.syntax_highlight import SyntaxHighlighter
from editor.model.text_model import EditorModel
from editor.view.curses_adapter import IViewAdapter


class EditorView(View):
    def __init__(self, model: EditorModel, adapter: IViewAdapter):
        super().__init__(model, adapter)
        self.highlighter = SyntaxHighlighter(adapter)

    def update(self):
        self.adapter.clear()
        h, w = self.adapter.get_window_size()

        display_line_num = 0
        start_line = self.model.scroll_offset

        line_num_width = 0
        if self.model.show_line_numbers:
            total_lines = len(self.model.lines)
            line_num_width = len(str(total_lines)) + 1
        text_width = w - line_num_width
        i = start_line
        while i < len(self.model.lines) and display_line_num < h - 1:
            line = self.model.lines[i]
            line_text = line.c_str()
            if self.model.show_line_numbers:
                line_num = i + 1
                line_num_str = f"{line_num:>{line_num_width - 1}} "
                self.adapter.addstr(display_line_num, 0, line_num_str)
            text_x = line_num_width if self.model.show_line_numbers else 0
            if self.model.syntax_highlight and line_text:
                segments = self.highlighter.highlight_line(line_text)
            else:
                segments = []
            start = 0
            line_len = len(line_text)
            while (start < line_len or line_len == 0) and display_line_num < h - 1:
                if line_len == 0:
                    self.adapter.addstr(display_line_num, text_x, "")
                    display_line_num += 1
                    break
                end = min(start + text_width, line_len)
                screen_text = line_text[start:end]
                if segments:
                    screen_segments = []
                    for seg_start, seg_end, token_type in segments:
                        if seg_end <= start or seg_start >= end:
                            continue
                        ss = max(seg_start, start) - start
                        se = min(seg_end, end) - start
                        if ss < se:
                            screen_segments.append((ss, se, token_type))
                    if screen_segments:
                        self._draw_with_highlight(
                            screen_text,
                            screen_segments,
                            display_line_num,
                            text_x
                        )
                    else:
                        self.adapter.addstr(display_line_num, text_x, screen_text)
                else:
                    self.adapter.addstr(display_line_num, text_x, screen_text)
                display_line_num += 1
                start += text_width
            i += 1

            filename_only = None
            if self.model.filename:
                filename_only = self.model.filename.split('\\')[-1]
            status = f" {self.model.mode} | {filename_only or '[No Name]'} | {self.model.cursor_y + 1}/{len(self.model.lines)}:{self.model.cursor_x + 1} "
            if self.model.mode == "COMMAND":
                status += f":{self.model.command_buffer.c_str()}_"
            elif self.model.mode == "SEARCH":
                direction_symbol = "/" if self.model.search_direction == 1 else "?"
                status += f"{direction_symbol}{self.model.search_pattern.c_str()}_"
            if self.model.modified:
                status += " [+]"
            if self.model.last_error.size() > 0:
                error_display = self.model.last_error.c_str()[:w // 2]
                status += f" [{error_display}]"
            self.adapter.draw_status_bar(status)

        screen_y, screen_x = self.calculate_screen_position()
        self.adapter.move(screen_y, screen_x)
        self.adapter.refresh()

    def _draw_with_highlight(self, text: str, segments: list,
                             line_num: int, x: int):
        if not text or not segments:
            self.adapter.addstr(line_num, x, text)
            return
        last_pos = 0
        for seg_start, seg_end, token_type in segments:
            if seg_start > last_pos:
                before_seg = text[last_pos:seg_start]
                if before_seg:
                    self.adapter.addstr(line_num, x + last_pos, before_seg)
            seg_text = text[seg_start:seg_end]
            if seg_text:
                color_attr = self.highlighter.get_color(token_type)
                self.adapter.addstr(line_num, x + seg_start, seg_text, color_attr)
            last_pos = seg_end
        if last_pos < len(text):
            after_seg = text[last_pos:]
            if after_seg:
                self.adapter.addstr(line_num, x + last_pos, after_seg)

    def calculate_screen_position(self):
        h, w = self.adapter.get_window_size()
        line_num_width = 0
        if self.model.show_line_numbers:
            total_lines = len(self.model.lines)
            line_num_width = len(str(total_lines)) + 1
        text_width = w - line_num_width

        screen_y = 0
        start_line = self.model.scroll_offset
        for i in range(start_line, self.model.cursor_y):
            line_len = self.model.lines[i].size()
            if line_len == 0:
                screen_y += 1
            else:
                screen_y += (line_len + text_width - 1) // text_width
            if screen_y >= h - 1:
                return h - 2, line_num_width

        screen_y += self.model.cursor_x // text_width
        screen_x = line_num_width + (self.model.cursor_x % text_width)

        return min(screen_y, h - 2), screen_x
from typing import List, Tuple
import re

from editor.view.curses_adapter import IViewAdapter


class SyntaxHighlighter:
    KEYWORD = 1
    STRING = 2
    COMMENT = 3
    NUMBER = 4
    FUNCTION = 5
    MAGIC = 6

    KEYWORDS = {
        'False', 'None', 'True', 'and', 'as', 'assert', 'async', 'await',
        'break', 'class', 'continue', 'def', 'del', 'elif', 'else', 'except',
        'finally', 'for', 'from', 'global', 'if', 'import', 'in', 'is',
        'lambda', 'nonlocal', 'not', 'or', 'pass', 'raise', 'return',
        'try', 'while', 'with', 'yield'
    }
    BUILTINS = {
        'abs', 'all', 'any', 'ascii', 'bin', 'bool', 'breakpoint', 'bytearray',
        'bytes', 'callable', 'chr', 'classmethod', 'compile', 'complex',
        'delattr', 'dict', 'dir', 'divmod', 'enumerate', 'eval', 'exec',
        'filter', 'float', 'format', 'frozenset', 'getattr', 'globals',
        'hasattr', 'hash', 'help', 'hex', 'id', 'input', 'int', 'isinstance',
        'issubclass', 'iter', 'len', 'list', 'locals', 'map', 'max', 'memoryview',
        'min', 'next', 'object', 'oct', 'open', 'ord', 'pow', 'print', 'property',
        'range', 'repr', 'reversed', 'round', 'set', 'setattr', 'slice',
        'sorted', 'staticmethod', 'str', 'sum', 'super', 'tuple', 'type',
        'vars', 'zip', '__import__'
    }
    SPECIAL = {
        'self', 'cls',
        '__init__', '__new__', '__del__',
        '__repr__', '__str__', '__format__', '__bytes__',
        '__eq__', '__ne__', '__lt__', '__le__', '__gt__', '__ge__',
        '__hash__', '__bool__',
        '__getattr__', '__getattribute__', '__setattr__', '__delattr__', '__dir__',
        '__get__', '__set__', '__delete__',
        '__len__', '__length_hint__', '__getitem__', '__setitem__', '__delitem__',
        '__iter__', '__reversed__', '__contains__',
        '__add__', '__sub__', '__mul__', '__matmul__', '__truediv__', '__floordiv__',
        '__mod__', '__divmod__', '__pow__', '__lshift__', '__rshift__',
        '__and__', '__xor__', '__or__',
        '__radd__', '__rsub__', '__rmul__', '__rmatmul__', '__rtruediv__', '__rfloordiv__',
        '__rmod__', '__rdivmod__', '__rpow__', '__rlshift__', '__rrshift__',
        '__rand__', '__rxor__', '__ror__',
        '__iadd__', '__isub__', '__imul__', '__imatmul__', '__itruediv__', '__ifloordiv__',
        '__imod__', '__ipow__', '__ilshift__', '__irshift__',
        '__iand__', '__ixor__', '__ior__',
        '__neg__', '__pos__', '__abs__', '__invert__',
        '__complex__', '__int__', '__float__', '__index__',
        '__round__', '__trunc__', '__floor__', '__ceil__',
        '__enter__', '__exit__',
        '__call__', '__await__', '__aiter__', '__anext__', '__aenter__', '__aexit__',
        '__doc__', '__name__', '__qualname__', '__module__', '__dict__', '__weakref__',
        '__annotations__', '__slots__',
    }


    def __init__(self, adapter: IViewAdapter = None):
        self.patterns = [
            (r'#.*$', self.COMMENT),
            #    \b - граница слова (начало/конец)
            #    \d+ - одна или более цифр
            #    \.? - необязательная точка
            #    \d* - ноль или более цифр после точки
            #    (?:[eE][+-]?\d+)? - необязательная экспонента
            (r'\b\d+\.?\d*(?:[eE][+-]?\d+)?\b', self.NUMBER),
            #    " - открывающая кавычка
            #    [^"\\]* - любые символы, кроме " и \
            #    (?:\\.[^"\\]*)* - экранированные символы и остальное
            #    " - закрывающая кавычка
            (r'"[^"\\]*(?:\\.[^"\\]*)*"', self.STRING),
            (r"'[^'\\]*(?:\\.[^'\\]*)*'", self.STRING),
            (r'"""[\s\S]*?"""', self.STRING),
            (r"'''[\s\S]*?'''", self.STRING),
        ]
        self.adapter = adapter

    def get_color(self, type: int):
        if self.adapter and self.adapter.has_colors():
            color_map = {
                1: self.adapter.COLOR_KEYWORD,
                2: self.adapter.COLOR_STRING,
                3: self.adapter.COLOR_COMMENT,
                4: self.adapter.COLOR_NUMBER,
                5: self.adapter.COLOR_FUNCTION,
                6: self.adapter.COLOR_MAGIC
            }
            color_num = color_map.get(type, 0)
            return self.adapter.color_pair(color_num) if color_num else 0
        return 0

    def highlight_line(self, text: str) -> List[Tuple[int, int, int]]:
        #segment: (начало, конец, цвет)
        if not text:
            return []
        segments = []
        processed = [False] * len(text)

        for pattern, token_type in self.patterns:
            for match in re.finditer(pattern, text):
                start, end = match.span() # возвращает начальную и конечную позицию совпадения
                for i in range(start, end):
                    processed[i] = True
                segments.append((start, end, token_type))

        i = 0
        n = len(text)
        while i < n:
            if processed[i]:
                i += 1
                continue
            # Идентификаторы (имена переменных, функций и т.д.)
            if text[i].isalpha() or text[i] == '_':
                start = i
                while i < n and (text[i].isalnum() or text[i] == '_'):
                    i += 1
                word = text[start:i]
                if word in self.KEYWORDS:
                    segments.append((start, i, self.KEYWORD))
                elif word in self.BUILTINS:
                    segments.append((start, i, self.FUNCTION))
                elif word in self.SPECIAL:
                    segments.append((start, i, self.MAGIC))
                continue
            i += 1
        # Сортируем сегменты по начальной позиции (чтоб вложенные правильно отображать)
        segments.sort(key=lambda x: x[0])
        return segments


class CommandHistory:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance.commands = []
            cls._instance.states = []
        return cls._instance

    def add(self, cmd, model_state=None):
        if cmd.__class__.__name__ == 'UndoCommand':
            return
        self.commands.append(cmd)
        self.states.append(model_state)

    def get_last_command(self):
        if self.commands:
            return self.commands.pop(), self.states.pop()
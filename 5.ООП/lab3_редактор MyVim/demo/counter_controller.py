from mvc_framework.base import Controller

class CounterController(Controller):

    def run(self):
        self.model.notify()
        while self.running:
            key = self.adapter.get_key()
            if key == ord('q') or key == ord('Q'):
                self.stop()
            elif key in (self.adapter.UP, ord('+')):
                self.model.increment()
            elif key in (self.adapter.DOWN, ord('-')):
                self.model.decrement()
            elif key in (ord('r'), ord('R')):
                self.model.reset()

    def stop(self):
        self.running = False


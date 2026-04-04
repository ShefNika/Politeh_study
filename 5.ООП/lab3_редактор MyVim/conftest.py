import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

my_string_path = os.path.join(os.path.dirname(__file__), 'MyString.pyd')
if os.path.exists(my_string_path):
    print(f"MyString.pyd found at: {my_string_path}")
else:
    print(f"MyString.pyd not found at: {my_string_path}")
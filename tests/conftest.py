"""Shared test setup: never read or write the user's real GUI preferences.

The global "default computing kernel" (avas.engines.read_preference) is read from the GUI preferences
file by every kernel choice, including ``avas run`` children of the tests; without this, a preference
saved in the developer's own gui.json would change which kernel the tests run.  Test modules that need
their own file still set AVAS_GUI_SETTINGS themselves.
"""
import os

os.environ.setdefault("AVAS_GUI_SETTINGS", os.path.join(os.path.dirname(os.path.abspath(__file__)), "_output",
                                                        "gui-settings.json"))

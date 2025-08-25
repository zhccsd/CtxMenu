# CtxMenu

## Purpose

CtxMenu is a tool designed to let you easily build a custom submenu and add it to the Windows right-click context menu. All you need to do is set up a properly formatted `.xml` file to define their menu items and actions—no coding required.

## Build Instructions

### Prerequisites
- Visual Studio (2019 or later recommended)
- CMake

### Steps
1. Open a terminal and navigate to the project directory.
2. Run the following command to generate Visual Studio project files:
	```
	cmake -S . -B build
	```
3. Open the generated solution file (`build/CtxMenuDLL.sln`) in Visual Studio.
4. Build `INSTALL` from Visual Studio (choose Debug or Release as needed).


## How to Use

### 1. Installation

After building CtxMenu, locate `install.bat` in the output directory. Double-click `install.bat` to register the context menu extension. To uninstall, use `uninstall.bat` in the same directory.

### 2. Configuration

Create an XML file, or edit `sample.xml` to define your submenu and actions.

### 3. Usage

Once installed and configured, right-click a directory's background in Windows Explorer. Your custom submenu will appear in the context menu, populated according to your XML configuration.

You can update the XML file at any time; changes will be reflected the next time the context menu is opened.

At present, the custom context menu is only available when right-clicking on a directory’s background in Windows Explorer.


## Variables

Variables can be used in any string attribute of the `<action>` tag in your XML configuration. They will be replaced automatically when the menu is loaded. All variables are case-sensitive.

### Environment Variables

You can use standard Windows environment variables, such as:
- `%APPDATA%`
- `%LOCALAPPDATA%`
- `%USERPROFILE%`

To see the full list of available environment variables, run `set` in the Command Prompt.

### Built-in Variables

CtxMenu provides several built-in variables for convenience:
- `${CTXMENU_HOME_PATH}` — The directory containing the currently loaded `CtxMenu.dll` (as loaded by `explorer.exe`).
- `${CTXMENU_CONFIG_PATH}` — The directory where CtxMenu looks for XML configuration files.
- `${CTXMENU_PWD}` — The current directory where the context menu was opened. Available only when right-clicking on a directory background or a single directory.

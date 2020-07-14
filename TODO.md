# PandEdit TODO

### Bug Fixes
- When typing just before a function definition, the letters typed show up in the information

### Features
- UTF-8 characters

- Compile buffer should close automatically if compilation succeeded
	- This would rely on checking the exit code somehow
- Add visual indicator (asterisk) if a buffer has been modified
	- Option to revert buffer back to its state on disk
		- As proof of concept, make this a manual task first
		- Can make this automatically happen when a file changes externally

- Search forward and backward through the file
	- Ability to use a regex for the search
- Replace on text (using regex)

- Save all open files at once
	- Ask for where to save non-file-visiting buffers
	- On exit, ask if buffers should be saved

- Automatically pair parens, braces, and brackets
	- When inserting a newline between braces, add proper formatting

- Project file
	- Can contain frame layout information
	- Specify include directories
		- Should find functions in all files
	- Can specify a compile command

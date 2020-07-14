# PandEdit TODO

### Bug Fixes
- When typing just before a function definition, the letters typed show up in the information

### Features
- UTF-8 characters

- Compile buffer should close automatically if compilation succeeded
	- This would rely on checking the exit code somehow

- Automatically revert if file was changed externally

- Search forward and backward through the file
	- Ability to use a regex for the search
- Replace on text (using regex)

- Check if any buffers need saving before asking on quit

- Automatically pair parens, braces, and brackets
	- When inserting a newline between braces, add proper formatting

- Project file
	- Can contain frame layout information
	- Specify include directories
		- Should find functions in all files
	- Can specify a compile command

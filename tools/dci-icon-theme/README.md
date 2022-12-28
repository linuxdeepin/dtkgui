# dci-icon-theme
````
Usage: dci-icon-theme [options] ~/dci-png-icons

Options:
  -m, --match <wildcard palette>  Give wildcard rules on search icon files,
                                  Each eligible icon will be packaged to a dci
                                  file, If the icon have the dark mode, it needs
                                  to store the dark icon file at "dark/"
                                  directory relative to current icon file, and
                                  the file name should be consistent.
  -o, --output <directory>        Save the *.dci files to the given directory.
  -s, --symlink <csv file>        Give a csv file to create symlinks for the
                                  output icon file.
  --fix-dark-theme                Create symlinks from light theme for dark
                                  theme files.
  -h, --help                      Displays help on commandline options.

  -v, --version                   Displays version information.

Arguments:
  source                          Search the given directory and it's
                                  subdirectories, get the files conform to rules
                                  of --match.
````
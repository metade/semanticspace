# Loads mkmf which is used to make makefiles for Ruby extensions
require 'mkmf'

# Give it a name
extension_name = 'semanticspace'
# The destination
dir_config(extension_name)
# Include libraries
find_library('svd', '')
find_library('SemanticSpace', '')
# Do the work
create_makefile(extension_name)
find -type f -name "*.C" -exec sed -i "s/#include <snprintf.h>//" {} \; -print
find -type f -name "*.h" -exec sed -i "s/#include <snprintf.h>//" {} \; -print
find -type f -name "*.c" -exec sed -i "s/#include <snprintf.h>//" {} \; -print
find -type f -name "*.C" -exec sed -i "s/#include \"snprintf.h\"//" {} \; -print
find -type f -name "*.h" -exec sed -i "s/#include \"snprintf.h\"//" {} \; -print
find -type f -name "*.c" -exec sed -i "s/#include \"snprintf.h\"//" {} \; -print
find -type f -name "*.h" -exec sed -i "s/SNPRINTF/snprintf/" {} \; -print
find -type f -name "*.C" -exec sed -i "s/SNPRINTF/snprintf/" {} \; -print
find -type f -name "*.c" -exec sed -i "s/SNPRINTF/snprintf/" {} \; -print
find -type f -name "*.code" -exec sed -i "s/SNPRINTF/snprintf/" {} \; -print

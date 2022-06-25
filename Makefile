TARGET = ezmem.h

SHELL = /bin/bash

SRCS = \
	init.h \
	utils_io.h \
	utils_file.h \
	utils_nbr.h \
	constructor.h \
	destructor.h \
	wrap.h \

SRC = $(addprefix srcs/,$(SRCS))

VPATH := srcs/

all : rm header $(TARGET) footer
	@printf "\e[32;1m+++ CONCAT [$(SRC)] +++\e[0m\n"

.PHONY : rm header $(TARGET) footer
.FORCE : ;

rm : .FORCE
	@rm -f $(TARGET)

header :
	@printf "#ifndef EZMEM\n" >> $(TARGET)
	@printf " #define EZMEM\n" >> $(TARGET)

footer :
	@printf "\n#endif //EZMEM\n" >> $(TARGET)

$(TARGET) :
	@for file in $$(echo "$(SRC)" | xargs);\
	do \
	 	echo >> $(TARGET); \
		echo //////////////////////////////////////////////////////////// $$file >> $(TARGET); \
		cat $$file >> $(TARGET); \
	done

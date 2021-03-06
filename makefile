LIBNAME = myframe
SUBFFIX	= .so
BINDIR	= bin
LIBDIR	= lib
OBJDIR	= obj
TMPDIR  = $(BINDIR) $(LIBDIR) $(OBJDIR)

CXX		= g++
FLAG    = -g
CCFLAG	= $(FLAG) -Wall -fPIC -D_DEBUG_
SOFLAG	= $(FLAG) -shared -o
APPFLAG = $(FLAG) -o
INCFLAG	= -I./ $(shell mysql_config --cflags)
SOLIBS  = $(shell mysql_config --libs)
APPLIBS = -L$(LIBDIR) -l$(LIBNAME) $(SOLIBS) -lpthread

TEST	= $(foreach d,$(shell ls test),test/$(d))
DIRS	= memory mysql socket thread httpd utils $(TEST)
SRCS	= $(foreach d,$(DIRS),$(wildcard $(d)/*.cpp))
OBJS	= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
SOOBJS	= $(filter-out $(OBJDIR)/test/%.o,$(OBJS))
APPOBJS	= $(filter $(OBJDIR)/test/%.o,$(OBJS))

#NULLSTR =
#SPACE	= $(NULLSTR) #end of the line
#VPATH	= $(subst $(SPACE),:,$(DIRS))

TARGET	= $(LIBDIR)/lib$(LIBNAME)$(SUBFFIX)

all : dir lib app
lib : $(TARGET)

dir:
	@-rm -rf $(TMPDIR)
	@-mkdir $(TMPDIR)
	@-mkdir -p $(foreach d, $(DIRS), $(OBJDIR)/$(d))

$(TARGET) : $(SOOBJS)	
	$(CXX) $(SOFLAG) $@ $^ $(SOLIBS)

app: $(APPOBJS)
	$(foreach a,$(APPOBJS),$(CXX) $(APPFLAG) $(BINDIR)/$(basename $(notdir $(a))) $(a) $(APPLIBS);)
	
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CCFLAG) $(INCFLAG) -c $< -o $@
$(OBJDIR)/%.o : %.c
	$(CXX) $(CCFLAG) $(INCFLAG) -c $< -o $@

.PHONY : clean
clean :
	-rm -rf $(TMPDIR)

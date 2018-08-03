CXX		= g++
CCFLAG	= -g -D_DEBUG_ -fPIC
SOFLAG	= -shared -o
LKFLAG	= -o
INCFLAG	= -I./
LIBDIR	= lib
OBJDIR	= obj
BINDIR	= bin

DIRS	= socket thread utils test
SRCS	= $(foreach d,$(DIRS),$(wildcard $(d)/*.cpp))
OBJS	= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

SOOBJS	= $(filter-out $(OBJDIR)/test/%.o,$(OBJS))
APPOBJS	= $(filter $(OBJDIR)/test/%.o,$(OBJS))
FRAME	= frame
TARGET	= $(LIBDIR)/lib$(FRAME).so

NULLSTR =
SPACE	= $(NULLSTR) #end of the line
VPATH	= $(subst $(SPACE),:,$(DIRS))

all : MKDIR $(TARGET) MKAPP

MKDIR:
	mkdir -p $(LIBDIR) $(BINDIR)
	mkdir -p $(foreach d, $(DIRS), $(OBJDIR)/$(d))

$(TARGET) : $(SOOBJS)	
	$(CXX) $(SOFLAG) $@ $^

MKAPP: $(APPOBJS)
	$(foreach a,$(APPOBJS),\
	$(CXX) $(LKFLAG) $(BINDIR)/t-$(basename $(notdir $(a))) $(a) -L$(LIBDIR) -l$(FRAME) -lpthread;)
	
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CCFLAG) $(INCFLAG) -c $< -o $@
$(OBJDIR)/%.o : %.c
	$(CXX) $(CCFLAG) $(INCFLAG) -c $< -o $@

.PHONY : clean
clean :
	-rm -rf $(OBJDIR)/* $(TARGET) $(BINDIR)/*
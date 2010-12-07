#########################################################################
#
# This is the master makefile.
# Here we put all the top-level make targets, platform-independent
# rules, etc.
#
# Run 'make help' to list helpful targets.
#
#########################################################################


.PHONY: all debug profile clean realclean nuke doxygen

working_dir	:= ${shell pwd}
INSTALLDIR=${working_dir}

# Figure out which architecture we're on
include ${working_dir}/src/make/detectplatform.mk

# Presence of make variables DEBUG and PROFILE cause us to make special
# builds, which we put in their own areas.
ifdef DEBUG
    variant +=.debug
endif
ifdef PROFILE
    variant +=.profile
endif

MY_MAKE_FLAGS ?=
MY_CMAKE_FLAGS ?= -g3

# Site-specific build instructions
ifneq (${shell uname -n | grep imageworks},)
include ${working_dir}/site/spi/Makefile-bits
endif

ifneq (${VERBOSE},)
MY_MAKE_FLAGS += VERBOSE=${VERBOSE}
endif

ifneq (${USE_TBB},)
MY_CMAKE_FLAGS += -DUSE_TBB:BOOL=${USE_TBB}
endif

ifneq (${USE_LLVM},)
MY_CMAKE_FLAGS += -DUSE_LLVM:BOOL=${USE_LLVM}
endif

ifneq (${LLVM_DIRECTORY},)
MY_CMAKE_FLAGS += -DLLVM_CUSTOM:BOOL=1 -DLLVM_DIRECTORY:STRING=${LLVM_DIRECTORY}
endif

ifneq (${LLVM_VERSION},)
MY_CMAKE_FLAGS += -DLLVM_VERSION:STRING=${LLVM_VERSION}
endif

ifneq (${NAMESPACE},)
MY_CMAKE_FLAGS += -DOSL_NAMESPACE:STRING=${NAMESPACE}
endif

ifdef DEBUG
MY_CMAKE_FLAGS += -DCMAKE_BUILD_TYPE:STRING=Debug
endif

ifneq (${MYCC},)
MY_CMAKE_FLAGS += -DCMAKE_C_COMPILER:STRING=${MYCC}
endif
ifneq (${MYCXX},)
MY_CMAKE_FLAGS += -DCMAKE_CXX_COMPILER:STRING=${MYCXX}
endif

#$(info MY_CMAKE_FLAGS = ${MY_CMAKE_FLAGS})
#$(info MY_MAKE_FLAGS = ${MY_MAKE_FLAGS})


# Set up variables holding the names of platform-dependent directories --
# set these after evaluating site-specific instructions
top_build_dir := build
build_dir     := ${top_build_dir}/${platform}${variant}
top_dist_dir  := dist
dist_dir      := ${top_dist_dir}/${platform}${variant}

$(info dist_dir = ${dist_dir})
$(info INSTALLDIR = ${INSTALLDIR})


#########################################################################




#########################################################################
# Top-level documented targets

all: dist

# 'make debug' is implemented via recursive make setting DEBUG
debug:
	${MAKE} DEBUG=1 --no-print-directory

# 'make profile' is implemented via recursive make setting PROFILE
profile:
	${MAKE} PROFILE=1 --no-print-directory

# 'make cmakesetup' constructs the build directory and runs 'cmake' there,
# generating makefiles to build the project.  For speed, it only does this when
# ${build_dir}/Makefile doesn't already exist, in which case we rely on the
# cmake generated makefiles to regenerate themselves when necessary.
cmakesetup:
	@ (if [ ! -e ${build_dir}/Makefile ] ; then \
		cmake -E make_directory ${build_dir} ; \
		cd ${build_dir} ; \
		cmake -DCMAKE_INSTALL_PREFIX=${INSTALLDIR}/${dist_dir} \
			${MY_CMAKE_FLAGS} -DBOOST_ROOT=${BOOST_HOME} \
			../../src ; \
	 fi)

# 'make cmake' does a basic build (after first setting it up)
cmake: cmakesetup
	( cd ${build_dir} ; ${MAKE} ${MY_MAKE_FLAGS} )

# 'make cmakeinstall' builds everthing and installs it in 'dist'
cmakeinstall: cmake
	( cd ${build_dir} ; ${MAKE} ${MY_MAKE_FLAGS} install )

# 'make dist' is just a synonym for 'make cmakeinstall'
dist : cmakeinstall

# 'make test' does a full build and then runs all tests
test: cmakeinstall
	( cd ${build_dir} ; ${MAKE} ${MY_MAKE_FLAGS} test )

# 'make package' builds everything and then makes an installable package 
# (platform dependent -- may be .tar.gz, .sh, .dmg, .rpm, .deb. .exe)
package: cmakeinstall
	( cd ${build_dir} ; ${MAKE} ${MY_MAKE_FLAGS} package )

# 'make package_source' makes an installable source package 
# (platform dependent -- may be .tar.gz, .sh, .dmg, .rpm, .deb. .exe)
package_source: cmakeinstall
	( cd ${build_dir} ; ${MAKE} ${MY_MAKE_FLAGS} package_source )

#clean: testclean
# 'make clean' clears out the build directory for this platform
clean:
	cmake -E remove_directory ${build_dir}

# 'make realclean' clears out both build and dist directories for this platform
realclean: clean
	cmake -E remove_directory ${dist_dir}

# 'make nuke' blows away the build and dist areas for all platforms
nuke:
	cmake -E remove_directory ${top_build_dir}
	cmake -E remove_directory ${top_dist_dir}

doxygen:
	doxygen src/doc/Doxyfile

#testclean : ${all_tests}
#	@ for f in ${all_tests} ; do \
#	    ( cd $$f ; \
#	      echo "Cleaning test $$f " ; \
#	      ./run.py -c ; \
#	    ) \
#	done

#########################################################################



# 'make help' prints important make targets
help:
	@echo "Targets:"
	@echo "  make              Build optimized binaries and libraries in ${dist_dir},"
	@echo "                        temporary build files in ${build_dir}"
	@echo "  make debug        Build unoptimized with symbols in ${dist_dir}.debug,"
	@echo "                        temporary build files in ${build_dir}.debug"
	@echo "  make profile      Build for profiling in ${dist_dir}.profile,"
	@echo "                        temporary build files in ${build_dir}.profile"
	@echo "  make clean        Remove the temporary files in ${build_dir}"
	@echo "  make realclean    Remove both ${build_dir} AND ${dist_dir}"
	@echo "  make nuke         Remove ALL of build and dist (not just ${platform})"
	@echo "  make test         Run all tests"
	@echo "  make doxygen      Build the Doxygen docs in ${top_build_dir}/doxygen"
	@echo ""
	@echo "Helpful modifiers:"
	@echo "  make VERBOSE=1 ...          Show all compilation commands"
	@echo "  make MYCC=xx MYCXX=yy ...   Use custom compilers"
	@echo "  make USE_TBB=0 ...          Don't use TBB"
	@echo "  make USE_LLVM=[0|1] ...     Use/don't use LLVM"
	@echo "  make NAMESPACE=name         Wrap everything in another namespace"
	@echo ""

# Copyright (c) 2014, AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
import sys
import os
sys.path.append('tools')
import codegen

Import('env')
if 'cpp' not in env['bindings']:
    print('Not building datadriven_api because cpp was not specified in bindings')
    Return()

if env.has_key('_ALLJOYNCORE_'):
    from_alljoyn_core=1
else:
    from_alljoyn_core=0

# Indicate that this SConscript file has been loaded already
env['_DATADRIVEN_'] = True

if env['CXX'][:5] == 'clang':
    #clang cannot always deal with gnu++0x
    env.Append(CXXFLAGS = '-D__STRICT_ANSI__')

if not env.has_key('_ALLJOYN_ABOUT_') and os.path.exists('../../core/alljoyn/services/about/SConscript'):
    env.SConscript('../../core/alljoyn/services/about/SConscript')

if not env.has_key('_ALLJOYN_SERVICES_COMMON_') and os.path.exists('../../services/base/services_common/SConscript'):
    env.SConscript('../../services/base/services_common/SConscript')

# DataDriven API uses AllJoyn Core, so always try to build it
if 'cpp' in env['bindings'] and not env.has_key('_ALLJOYNCORE_') and os.path.exists('../../core/alljoyn/alljoyn_core/SConscript'):
    env.SConscript('../../core/alljoyn/alljoyn_core/SConscript')

# Make DataDriven API library and header file paths available to the global environment
env.Append(CPPPATH = ['$DISTDIR/datadriven_cpp/inc'])
env.Append(LIBPATH = ['$DISTDIR/datadriven_cpp/lib'])

ddenv = env.Clone(tools = ['textfile', codegen.tool])

ddenv.Append(CCFLAGS = '-Werror')

def create_libpath(self):
    libs = map(lambda s: self.subst(s).replace('#', Dir('#').abspath + '/'), self['LIBPATH'])
    return ':'.join(libs)

AddMethod(Environment, create_libpath, "CreateLibPath")

# Make alljoyn C++ dist a sub-directory of the alljoyn dist.
ddenv['DD_DISTDIR'] = ddenv['DISTDIR'] + '/datadriven_cpp'
ddenv['DD_OBJDIR'] = ddenv['OBJDIR'] + '/datadriven_cpp'

# Set up DataDriven API dist directory
ddenv.Append(LIBS = ['alljoyn_about', 'alljoyn_services_common'])

# Add support for multiple build targets in the same work set.
ddenv.VariantDir('$DD_OBJDIR/lib', 'src', duplicate = 0)
ddenv.VariantDir('$DD_OBJDIR/samples', 'samples', duplicate = 0)
ddenv.VariantDir('$DD_OBJDIR/unit_test', 'unit_test', duplicate = 0)
ddenv.VariantDir('$DD_OBJDIR/test', 'test', duplicate = 0)

buildroot = ddenv.subst('build/${OS}/${CPU}/${VARIANT}')

# Build and install DataDriven API C++ binding
libs = ddenv.SConscript('$DD_OBJDIR/lib/SConscript', exports = {'env':ddenv})
ddenv.Install('$DD_DISTDIR/lib', libs)
ddenv.Install('$DD_DISTDIR/inc/datadriven', ddenv.Glob('inc/datadriven/*.h'))

# Remove this condition when codegen supports ddcpp
if from_alljoyn_core == 0:
    # Sample building
    samples = ddenv.SConscript('$DD_OBJDIR/samples/SConscript', exports = {'env':ddenv})
    ddenv.Install('$DD_DISTDIR/bin/samples', samples)

    # tests building (not installed)
    unittests = ddenv.SConscript('$DD_OBJDIR/unit_test/SConscript', exports= {'env':ddenv})
    tests = ddenv.SConscript('$DD_OBJDIR/test/SConscript', exports= {'env':ddenv})
else:
    print 'Not building datadriven_api samples due to codegen incompatibility'

# Whitespace policy (only when we don't build from alljoyn)
if from_alljoyn_core == 0:
    if ddenv['WS'] != 'off' and not ddenv.GetOption('clean'):
        import whitespace
        import time

        def wsbuild(target, source, env):
            print "Evaluating whitespace compliance..."
            curdir = os.path.abspath(os.path.dirname(wsbuild.func_code.co_filename))
            config = os.path.join(curdir, 'tools', 'ajuncrustify.cfg')
            print "Config:", config
            print "Note: enter 'scons -h' to see whitespace (WS) options"
            olddir = os.getcwd()
            os.chdir(curdir) #We only want to run uncrustify on datadriven_api
            time.sleep(1) #very dirty hack to prevent concurrent running of uncrustify
            retval = whitespace.main([env['WS'], config])
            os.chdir(olddir)
            return retval

        ddenv.Command('#/ws_dd', Dir('$DD_DISTDIR'), wsbuild)
    if ddenv['WS'] != 'off':
        ddenv.Clean(ddenv.File('SConscript'), ddenv.File('#/whitespace.db'))

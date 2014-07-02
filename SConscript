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

if 'cpp' in env['bindings'] and not env.has_key('_ALLJOYNCORE_') and os.path.exists('../../core/alljoyn/alljoyn_core/SConscript'):
    env.SConscript('../../core/alljoyn/alljoyn_core/SConscript')


# Make config library and header file paths available to the global environment
env.Append(LIBPATH = '$DISTDIR/datadriven/lib');
env.Append(CPPPATH = '$DISTDIR/datadriven/inc');

ddenv = env.Clone(tools = ['textfile', codegen.tool])

ddenv.Append(CCFLAGS = '-Werror')

def create_libpath(self):
    libs = map(lambda s: self.subst(s).replace('#', Dir('#').abspath + '/'), self['LIBPATH'])
    return ':'.join(libs)

AddMethod(ddenv, create_libpath, "CreateLibPath")

# Make alljoyn C++ dist a sub-directory of the alljoyn dist.
ddenv['DD_DISTDIR'] = env['DISTDIR'] + '/datadriven'

ddenv.Append(LIBS = ['alljoyn_about', 'alljoyn_services_common'])

buildroot = ddenv.subst('build/${OS}/${CPU}/${VARIANT}')

# put this in cpp later
ddenv.Install('$DD_DISTDIR/inc/datadriven', ddenv.Glob('inc/datadriven/*.h'))
ddenv.Append(CPPPATH = ['$DD_DISTDIR/inc/'])
ddenv.Install('$DD_DISTDIR/lib', ddenv.SConscript('src/SConscript', exports = ['ddenv'], variant_dir=buildroot+'/lib', duplicate=0))

# Remove this condition when codegen supports ddcpp
if from_alljoyn_core == 0:
    # Sample building
    for sample in Glob('samples/*', strings=True):
        ddenv.Install('$DD_DISTDIR/bin/samples', ddenv.SConscript(sample + '/SConscript', exports=['ddenv'], variant_dir=buildroot+'/sample/'+sample, duplicate=0))
    
    # System test building (are not installed)
    for systest in Glob('test/system/*', strings=True):
        testdir = buildroot+'/'+systest
        ddenv.SConscript(systest + '/SConscript', exports=['ddenv'], variant_dir=testdir, duplicate=0)

    # Unit test building (are not installed)
    ddenv.SConscript('test/unit/SConscript', exports=['ddenv'], variant_dir=buildroot+'/test/unit', duplicate=0)
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
    

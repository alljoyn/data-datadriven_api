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

# Library path creation for run scripts
def create_libpath(self):
    libs = map(lambda s: self.subst(s).replace('#', Dir('#').abspath + '/'), self['LIBPATH'])
    return ':'.join(libs)

AddMethod(Environment, create_libpath, "CreateLibPath")

# Main build code
env_aj_root = os.environ['AJ_ROOT'] if 'AJ_ROOT' in os.environ else '#../..'

vars = Variables(None, ARGUMENTS)
vars.Add('OS', 'Target OS', 'linux') # no other options for now
vars.Add(EnumVariable('CPU', 'Target CPU', 'x86_64', allowed_values=('x86', 'x86_64')))
vars.Add(EnumVariable('VARIANT', 'Build variant', 'debug', allowed_values=('debug', 'release')))
vars.Add('AJREPOPATH', 'Root path of the alljoyn repositories', env_aj_root)
vars.Add('AJCOREPATH', 'Root path of the alljoyn-core repository', '${AJREPOPATH}/core/alljoyn')
vars.Add(EnumVariable('WS', 'Whitespace Policy Checker', 'check', allowed_values=('check', 'detail', 'fix', 'off')))
vars.Add('CXX', 'C++ compiler to use')
vars.Add(EnumVariable('BR', 'Have bundled router built-in for tests and samples', 'off', allowed_values=('on', 'off'))) #Note the default value is off, unlike core AJN
vars.Add(BoolVariable('ENABLE_GCOV', 'Compile with gcov support', 'no'))

ajroot = '${AJCOREPATH}/build/linux/${CPU}/${VARIANT}/dist'
ajcommon = '${AJCOREPATH}/common'
ajroot_cpp = ajroot + '/cpp'
ajservicescommon = ajroot + '/services_common'

env = Environment(tools = ['default', 'textfile', codegen.tool],
                  variables = vars)

Help(vars.GenerateHelpText(env))

# compiling
env.Append(CCFLAGS = ['-Wall', '-Werror', '-Wno-long-long', '-Wno-deprecated'])
env.Append(CXXFLAGS = ['-std=c++0x', '-fno-exceptions'])
env.Append(CPPPATH = [ajroot_cpp + '/inc', ajcommon + '/inc', ajservicescommon + '/inc',  '#inc'])
env.Append(CPPDEFINES = ['QCC_OS_LINUX', 'QCC_OS_GROUP_POSIX'])
# linking
env.Append(LIBPATH = [ajroot_cpp + '/lib', ajroot + '/about/lib', ajroot + '/services_common/lib' ])
env.Append(LIBS = ['alljoyn', 'alljoyn_about', 'alljoyn_services_common', 'pthread'])

if env['VARIANT'] == 'debug':
    env.Append(CCFLAGS = ['-g', '-O0'])

if env['VARIANT'] == 'release':
    env.Append(CPPDEFINES = ['NDEBUG'])

if env['CPU'] == 'x86_64':
    env.Append(CCFLAGS = ['-m64', '-fPIC'])
    env.Append(LINKFLAGS = ['-m64'])

if env['ENABLE_GCOV'] and not env['CXX'] == 'clang':
    env.Append(CCFLAGS = ['-fprofile-arcs', '-ftest-coverage'])
    env.Append(LIBS = ['gcov'])

buildroot = env.subst('build/${OS}/${CPU}/${VARIANT}')

exports = ['env', 'ajroot', 'buildroot']

# Library building
SConscript('src/SConscript', exports=exports, variant_dir=buildroot + '/lib', duplicate=0)

# Sample building
for sample in Glob('samples/*', strings=True):
    SConscript(sample + '/SConscript', exports=exports, variant_dir=buildroot + '/' + sample, duplicate=0)

# System test building
for systest in Glob('test/system/*', strings=True):
    SConscript(systest + '/SConscript', exports=exports, variant_dir=buildroot + '/' + systest, duplicate=0)

# Unit test building
SConscript('test/unit/SConscript', exports=exports, variant_dir=buildroot + '/test/unit', duplicate=0)

# Whitespace policy
if env['WS'] != 'off' and not env.GetOption('clean'):
    import whitespace

    def wsbuild(target, source, env):
        print "Evaluating whitespace compliance..."
        curdir = os.path.abspath(os.path.dirname(wsbuild.func_code.co_filename))
        config = os.path.join(curdir, 'tools', 'ajuncrustify.cfg')
        print "Config:", config
        print "Note: enter 'scons -h' to see whitespace (WS) options"
        return whitespace.main([env['WS'], config])

    ws = env.Command('#/ws', Dir('$DISTDIR'), wsbuild)
if env['WS'] != 'off':
    env.Clean(env.File('SConscript'), env.File('#/whitespace.db'))

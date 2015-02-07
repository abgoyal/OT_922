import test
import os
from os.path import join, dirname, exists
import platform
import utils

CCTEST_DEBUG_FLAGS = ['--enable-slow-asserts', '--debug-code', '--verify-heap']


class CcTestCase(test.TestCase):

  def __init__(self, path, executable, mode, raw_name, dependency, context):
    super(CcTestCase, self).__init__(context, path)
    self.executable = executable
    self.mode = mode
    self.raw_name = raw_name
    self.dependency = dependency

  def GetLabel(self):
    return "%s %s %s" % (self.mode, self.path[-2], self.path[-1])

  def GetName(self):
    return self.path[-1]

  def BuildCommand(self, name):
    serialization_file = join('obj', 'test', self.mode, 'serdes')
    serialization_file += '_' + self.GetName()
    serialization_option = '--testing_serialization_file=' + serialization_file
    result = [ self.executable, name, serialization_option ]
    if self.mode == 'debug':
      result += CCTEST_DEBUG_FLAGS
    return result

  def GetCommand(self):
    return self.BuildCommand(self.raw_name)

  def Run(self):
    if self.dependency != '':
      dependent_command = self.BuildCommand(self.dependency)
      output = self.RunCommand(dependent_command)
      if output.HasFailed():
        return output
    return test.TestCase.Run(self)


class CcTestConfiguration(test.TestConfiguration):

  def __init__(self, context, root):
    super(CcTestConfiguration, self).__init__(context, root)

  def GetBuildRequirements(self):
    return ['cctests']

  def ListTests(self, current_path, path, mode):
    executable = join('obj', 'test', mode, 'cctest')
    if utils.IsWindows():
      executable += '.exe'
    output = test.Execute([executable, '--list'], self.context)
    if output.exit_code != 0:
      print output.stdout
      print output.stderr
      return []
    result = []
    for test_desc in output.stdout.strip().split():
      raw_test, dependency = test_desc.split('<')
      relative_path = raw_test.split('/')
      full_path = current_path + relative_path
      if dependency != '':
        dependency = relative_path[0] + '/' + dependency
      if self.Contains(path, full_path):
        result.append(CcTestCase(full_path, executable, mode, raw_test, dependency, self.context))
    return result

  def GetTestStatus(self, sections, defs):
    status_file = join(self.root, 'cctest.status')
    if exists(status_file):
      test.ReadConfigurationInto(status_file, sections, defs)


def GetConfiguration(context, root):
  return CcTestConfiguration(context, root)
require 'rake'
require 'rake/clean'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'
require 'fileutils'
include FileUtils

CLEAN.include [
  'ext/semanticspace/Makefile',
  'ext/semanticspace/*.{bundle,so,obj,pdb,lib,def,exp,o,log,dSYM}',
  '**/.*.sw?', '*.gem', '.config', 'pkg', 'test/*.llss', 'doc', 'lib'
]

desc "Does a full compile, test run, and build docs and gem"
task :default => [:compile, :test, :rdoc, :gem]

desc "Compiles all extensions"
task :compile => [:semanticspace] do
  if Dir.glob(File.join("lib","semanticspace.*")).length == 0
    STDERR.puts "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    STDERR.puts "Gem actually failed to build.  Your system is"
    STDERR.puts "NOT configured properly to build semanticspace."
    STDERR.puts "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    exit(1)
  end
end
task :semanticspace

desc "Run all the tests"
Rake::TestTask.new do |t|
  t.libs << "test"
  t.test_files = FileList['test/test_*.rb']
  t.verbose = true
end

Rake::RDocTask.new do |rdoc|
  rdoc.rdoc_dir = 'doc'
  rdoc.title = 'Semantic Space'
  rdoc.options += ['--quiet', '--inline-source']
  rdoc.main = "README"
  rdoc.rdoc_files.add ['README', 'ext/**/*.c']
end

spec = eval(File.open('semanticspace.gemspec').read)
Rake::GemPackageTask.new(spec) do |p|
  p.need_tar = true
  p.gem_spec = spec
end

extension = "semanticspace"
ext = "ext/semanticspace"
ext_so = "#{ext}/#{extension}.#{Config::CONFIG['DLEXT']}"
ext_files = FileList[
  "#{ext}/*.c",
  "#{ext}/*.h",
  "#{ext}/*.rl",
  "#{ext}/extconf.rb",
  "#{ext}/Makefile",
  "lib"
] 

task "lib" do
  directory "lib"
end

desc "Builds just the #{extension} extension"
task extension.to_sym => ["#{ext}/Makefile", ext_so ]

file "#{ext}/Makefile" => ["#{ext}/extconf.rb"] do
  Dir.chdir(ext) do ruby "extconf.rb" end
end

file ext_so => ext_files do
  Dir.chdir(ext) do
    sh(PLATFORM =~ /win32/ ? 'nmake' : 'make')
  end
  cp ext_so, "lib"
end

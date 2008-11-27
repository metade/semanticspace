Gem::Specification.new do |s|
  s.name = "semanticspace"
  s.version = "0.0.2"
  s.platform = Gem::Platform::RUBY
  s.has_rdoc = true
  s.date = "2008-11-27"
  s.summary = "Ruby bindings for libSemanticSpace"
  s.email = "metade@gmail.com"
  s.homepage = "http://github.com/metade/semanticspace"
  s.description = "Ruby bindings for libSemanticSpace, a library for the creation, modification and querying of semantic spaces."
  s.authors = ['Patrick Sinclair']
  s.files = ["README", "Rakefile", "semanticspace.gemspec", "ext/semanticspace/semanticspace.c"]
  s.extensions = ["ext/semanticspace/extconf.rb"]
  s.test_files = ["test/test_semanticspace.rb"]
end

require 'test/unit' 
require 'semanticspace'

include SemanticSpace

class SemanticSpaceTest < Test::Unit::TestCase 
  def test_semanticspace 
    t = SemanticSpace::SemanticSpace.new(1,2,3,'none') 
    assert_equal(Object, SemanticSpace::SemanticSpace.superclass) 
    assert_equal(SemanticSpace::SemanticSpace, t.class) 
  end 
  
  def test_index_term
    t = SemanticSpace::SemanticSpace.new(1,2,3,'none') 
    t.index_term('hello')
    terms = t.list_terms
    assert_equal(terms,['hello'])
  end
  
  def test_index_doc
    t = SemanticSpace::SemanticSpace.new(1,1,1,'none') 

    t.index_term('white')    
    t.index_term('text')    
    t.index_doc('test/white.tiff', true)
    t.index_doc('test/text.tiff', false)
    assert_equal(t.list_docs(true), ["test/white.tiff"])
    assert_equal(t.list_docs(false), ["test/text.tiff"])    
    t.add_term_occurrance('white','test/white.tiff',1.0,true)
    t.add_term_occurrance('text','test/text.tiff',1.0,false)
    
    t.train(0)
    t.project
    
    t.write('test/test.llss')
    s = SemanticSpace::read_semanticspace('test/test.llss')
    assert_equal(s.list_terms, ["text", "white"])
    
    assert(s)
  end
  
  def test_search_original_term
      s = SemanticSpace::read_semanticspace('test/test.llss')
      assert_equal(["text", "white"], s.list_terms)
      results = s.search_with_original_term('white')
      assert_equal(1, results.size)
      assert_equal('test/white.tiff', results.first.ident)
      assert_equal(1.0, results.first.similarity)      
  end
  
  def test_search_complex_query
    s = SemanticSpace::read_semanticspace('test/test.llss')
    query = SparseMatrix.new
    assert query
    
    doc_results = s.search_with_doc('test/white.tiff', true, TERM_SPACE, 10)
    
    query = s.construct_empty_query
    s.add_term_to_query(query, 'text', 1.0) # TODO: should return true?
    query_results = s.search_with_query(query, TERM_SPACE, 10)
    
    assert_equal doc_results, query_results
  end
  
  # def test_read_space
  #   t = SemanticSpace::read_semanticspace('test/CorelSpace.llss')
  #   terms = t.list_terms
  #   
  #   assert_equal(terms.size, 435)
  #   
  #   results = t.search_with_term('cliff', TERM_SPACE, 10)
  #   assert_equal(results.size, 435)
  # 
  #   results = t.search_with_doc('1004', true, TERM_SPACE, 10)
  #   assert_equal(results.size, 435)
  # 
  #   results = t.search_with_doc('1004', true, TERM_SPACE, 10, 3, '_bin[')
  #   assert_equal(results.size, 3)
  # end
  # 
  # def test_read_space_and_write_and_read
  #   t = SemanticSpace::read_semanticspace('test/CorelSpace.llss')
  #   t.write('test/test.llss')
  #   
  #   s = SemanticSpace::read_semanticspace('test/test.llss')
  #   terms = s.list_terms
  #   assert_equal(terms.size, 435)
  # end
end
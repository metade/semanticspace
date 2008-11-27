// Include the Ruby headers and goodies
#include "ruby.h"

#include "SemanticSpace.h"

// Defining a space for information and references about the module to be stored internally
VALUE rb_mSemanticSpace = Qnil;
VALUE rb_cSemanticSpace = Qnil;
VALUE rb_cSparseMatrix = Qnil;
VALUE rb_cSearchResult = Qnil;
VALUE rb_sSearchResultStruct = Qnil; 

// Prototype for the initialization method - Ruby calls this, not you
void Init_semanticspace();

// SemanticSpace wrapper
static void semanticspace_mark (SemanticSpace ss) {}
static void semanticspace_free (SemanticSpace ss) {
  SSFree(ss);
}
static VALUE ss_alloc(VALUE klass) {
  SemanticSpace ss = SSCreate(0, 0, 0, NULL);
  return Data_Wrap_Struct (klass, semanticspace_mark, semanticspace_free, ss);
  return 0;
}

// SparseMatrix wrapper
static void sparsematrix_mark (SparseMatrix sm) {}
static void sparsematrix_free (SparseMatrix sm) {
  SSFreeQuery(sm);
}


/* 
* call-seq: 
* SemanticSpace.new(n_terms, num_train_docs, num_docs, weighting_scheme) -> new_semantic_space
* 
* Create a new SemanticSpace with the specified size of terms a +n_terms+, n
*/
static VALUE ss_init(VALUE self, VALUE nterms, VALUE ntraindocs, VALUE ndocs, VALUE weighting_scheme) {
  // todo: should free current one?
  
  int _nterms = NUM2INT(nterms);
  int _ntraindocs = NUM2INT(ntraindocs);
  int _ndocs = NUM2INT(ndocs);
  char * _weighting_scheme = RSTRING(StringValue(weighting_scheme))->ptr;  
  
  SemanticSpace new_ss = SSCreate(_nterms, _ntraindocs, _ndocs, NULL);
  return Data_Wrap_Struct(rb_cSemanticSpace, semanticspace_mark, semanticspace_free, new_ss);
}

// SemanticSpace read/write methods
static VALUE ss_write(VALUE self, VALUE filepath) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  VALUE str = StringValue(filepath); 
  char * p = RSTRING(str)->ptr;
  
  SSWrite(ss, p);
  return Qnil;
}

// SemanticSpace training methods
static VALUE ss_index_term(VALUE self, VALUE term) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  VALUE _term = StringValue(term); 
  char * p = RSTRING(_term)->ptr;
  
  SSIndexTerm(ss, p);
  return Qnil;
}

static VALUE str_array_to_ruby(const char ** strings, int l) {
  VALUE array = rb_ary_new(); 
  int i;
  for (i=0; i<l; i++) {
    char * string = (char *) strings[i];
    VALUE t = rb_str_new2(string);
    rb_funcall(array, rb_intern("push"), 1, t);
  }
  return array;
}

static VALUE ss_list_indexed_terms(VALUE self, VALUE doc, VALUE isTraining) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  VALUE _doc = StringValue(doc); 
  char * p = RSTRING(_doc)->ptr;  
  
  int l = 0;    
  const char ** terms = SSListIndexedTerms(ss, p, isTraining, &l);
  VALUE array = str_array_to_ruby(terms, l); 
  free(terms);
  return array;
}

static VALUE ss_list_terms(VALUE self) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  int l = 0;    
  const char ** terms = SSListTerms(ss, &l);
  VALUE array = str_array_to_ruby(terms, l); 
  free(terms);
  return array;
}

static VALUE ss_index_document(VALUE self, VALUE doc, VALUE isTraining) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  VALUE _doc = StringValue(doc); 
  char * p = RSTRING(_doc)->ptr;
  
  SSIndexDocument(ss, p, isTraining);
  return Qnil;
}

static VALUE ss_list_documents(VALUE self, VALUE isTraining) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  int l = 0;    
  const char ** docs = SSListDocuments(ss, isTraining, &l);
  VALUE array = str_array_to_ruby(docs, l); 
  free(docs);
  return array;
}

static VALUE ss_add_term_occurrance(VALUE self, VALUE term, VALUE doc, VALUE weight, VALUE isTraining) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  char * _term = RSTRING(StringValue(term))->ptr;
  char * _doc = RSTRING(StringValue(doc))->ptr;
  double _weight = NUM2DBL(weight);
      
  SSAddTermOccurence(ss, _term, _doc, _weight, isTraining);
  return Qnil;
}

static VALUE ss_train(VALUE self, VALUE k) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  int _k = NUM2INT(k);
  SSTrain(ss, _k);
  
  return Qnil;
}

static VALUE ss_project(VALUE self) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  SSProject(ss);
  
  return Qnil;
}

// Search methods
static VALUE convert_results_to_ruby(SearchResult res, int size, char * match) {
  VALUE results = rb_ary_new(); 
  SearchResult r;
  
    for (r=res; r!=NULL; r=r->next) {
    int current_size = NUM2INT(rb_funcall(results, rb_intern("size"), 0));
    // break if we have reached the required size
    if (size!=-1 && current_size>=size)
      break;
    // don't add terms that match the 'match' string
    if (match==NULL || strstr(r->ident, match)==NULL) {
      VALUE rbVal = rb_struct_new(rb_sSearchResultStruct,
        rb_str_new2(r->ident), 
        rb_float_new(r->similarity), 
        r->type); 
      rb_funcall(results, rb_intern("push"), 1, rbVal);      
    } 
  }
  SSFreeSearchResult(&res);    
  return results;
}

static VALUE ss_search_with_term(VALUE self, VALUE term, VALUE searchSpace, VALUE K) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 

  const char * _term = RSTRING(StringValue(term))->ptr;
  int _searchSpace = NUM2INT(searchSpace);
  int _K = NUM2INT(K);

  SearchResult res = SSSearchWithTerm(ss, _term, _searchSpace, _K, 0);
  return convert_results_to_ruby(res, -1, NULL);
}

static VALUE ss_search_with_original_term(VALUE self, VALUE term) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 

  const char * _term = RSTRING(StringValue(term))->ptr;

  SearchResult res = SSSearchTrainingWithOriginalTerm(ss, _term);
  return convert_results_to_ruby(res, -1, NULL);
}

static VALUE ss_search_with_original_term_weighted(VALUE self, VALUE term, VALUE K) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 

  const char * _term = RSTRING(StringValue(term))->ptr;
  int _K = NUM2INT(K);

  SearchResult res = SSSearchTrainingWithOriginalTermWeighted(ss, _term, _K, 0);
  return convert_results_to_ruby(res, -1, NULL);
}

/* call-seq: 
* SemanticSpace.search_with_doc -> Array 
* 
* Return the results of a search for doc  
*/ 
static VALUE ss_search_with_doc(int argc, VALUE *argv, VALUE self) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss); 
  
  VALUE doc, isTraining, searchSpace, K, size, match;
  rb_scan_args(argc, argv, "42", &doc, &isTraining, &searchSpace, &K, &size, &match);
  
  const char * _doc = RSTRING(StringValue(doc))->ptr;
  bool _isTraining = isTraining;
  int _searchSpace = NUM2INT(searchSpace);
  int _K = NUM2INT(K);
  
  int _size = (size==Qnil) ? -1 : NUM2INT(size);
  char * _match = (match==Qnil) ? NULL : RSTRING(StringValue(match))->ptr;
  
  SearchResult res = SSSearchWithDoc(ss, _doc, _isTraining, _searchSpace, _K, 0);
  return convert_results_to_ruby(res, _size, _match);
}

static VALUE method_read(VALUE self, VALUE filepath) {
  VALUE str = StringValue(filepath); 
  char * p = RSTRING(str)->ptr;
  
  SemanticSpace new_ss = SSRead(p);
  return Data_Wrap_Struct(rb_cSemanticSpace, semanticspace_mark, semanticspace_free, new_ss);
}

static VALUE ss_construct_empty_query(VALUE self) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss);
  
  SparseMatrix query = SSConstructEmptyQuery(ss);
  
  return Data_Wrap_Struct(rb_cSparseMatrix, sparsematrix_mark, sparsematrix_free, query);
}

static VALUE ss_add_term_to_query(VALUE self, VALUE query, VALUE term, VALUE weight) {
  SemanticSpace ss;
  Data_Get_Struct(self, SemanticSpaceRec, ss);

  SparseMatrix _query;
  Data_Get_Struct(query, SparseMatrixRec, _query);
  char * _term = RSTRING(StringValue(term))->ptr;
  double _weight = NUM2DBL(weight);
  
  return SSAddTermToQuery(ss, _query, _term, _weight);
}

/* call-seq: 
* SemanticSpace.search_with_query -> Array
* 
* Return the results of a search for a pseudo-doc
*/ 
static VALUE ss_search_with_query(int argc, VALUE *argv, VALUE self) {
  SemanticSpace ss; 
  Data_Get_Struct(self, SemanticSpaceRec, ss);
  
  VALUE query, searchSpace, K, size, match;
  rb_scan_args(argc, argv, "32", &query, &searchSpace, &K, &size, &match);
  
  SparseMatrix _query;
  Data_Get_Struct(query, SparseMatrixRec, _query);
  int _searchSpace = NUM2INT(searchSpace);
  int _K = NUM2INT(K);
  
  int _size = (size==Qnil) ? -1 : NUM2INT(size);  
  
  SearchResult res = SSSearchWithQuery(ss, _query, _searchSpace, _K, 0);
  return convert_results_to_ruby(res, _size, NULL);
}



/* 
Interface to the Semantic Space library developed at the University of Southampton.
 */
void Init_semanticspace() {
  rb_mSemanticSpace = rb_define_module("SemanticSpace");
  rb_define_method(rb_mSemanticSpace, "read_semanticspace", method_read, 1);

  /* Combined space: training document space (i.e. documents with annotations) */
  rb_define_const(rb_mSemanticSpace, "TRAINING_DOCUMENT_SPACE", INT2NUM(TRAINING_DOCUMENT_SPACE)); 
  /* Combined space: document space (i.e. documents with NO annotations) */
  rb_define_const(rb_mSemanticSpace, "DOCUMENT_SPACE", INT2NUM(DOCUMENT_SPACE)); 
  /* Combined space: both training and document space */
  rb_define_const(rb_mSemanticSpace, "COMBINED_SPACE", INT2NUM(TRAINING_DOCUMENT_SPACE | DOCUMENT_SPACE));   
  /* Combined space: term space */
  rb_define_const(rb_mSemanticSpace, "TERM_SPACE", INT2NUM(TERM_SPACE)); 
  
  /* Search Result */
  rb_sSearchResultStruct = rb_struct_define("SearchResult", "ident", "similarity", "type", NULL); 
  
  /* SemanticSpace class */
  rb_cSemanticSpace = rb_define_class_under (rb_mSemanticSpace, "SemanticSpace", rb_cObject);
  rb_define_alloc_func(rb_cSemanticSpace, ss_alloc);
  rb_define_method(rb_cSemanticSpace, "initialize", ss_init, 4); 

  rb_define_method(rb_cSemanticSpace, "write", ss_write, 1); 

  rb_define_method(rb_cSemanticSpace, "index_term", ss_index_term, 1); 
  rb_define_method(rb_cSemanticSpace, "list_terms", ss_list_terms, 0); 
  rb_define_method(rb_cSemanticSpace, "list_indexed_terms", ss_list_indexed_terms, 2); 
  rb_define_method(rb_cSemanticSpace, "index_doc", ss_index_document, 2); 
  rb_define_method(rb_cSemanticSpace, "list_docs", ss_list_documents, 1);
  rb_define_method(rb_cSemanticSpace, "train", ss_train, 1); 
  rb_define_method(rb_cSemanticSpace, "project", ss_project, 0); 
  
  rb_define_method(rb_cSemanticSpace, "add_term_occurrance", ss_add_term_occurrance, 4); 
    
  rb_define_method(rb_cSemanticSpace, "search_with_term", ss_search_with_term, 3); 
  rb_define_method(rb_cSemanticSpace, "search_with_original_term", ss_search_with_original_term, 1); 
  rb_define_method(rb_cSemanticSpace, "search_with_original_term_weighted", ss_search_with_original_term_weighted, 2);   
  rb_define_method(rb_cSemanticSpace, "search_with_doc", ss_search_with_doc, -1);
  
  /* Querying with a complex object */
  rb_cSparseMatrix = rb_define_class_under (rb_mSemanticSpace, "SparseMatrix", rb_cObject);
  rb_define_method(rb_cSemanticSpace, "construct_empty_query", ss_construct_empty_query, 0);
  rb_define_method(rb_cSemanticSpace, "add_term_to_query", ss_add_term_to_query, 3);
  rb_define_method(rb_cSemanticSpace, "search_with_query", ss_search_with_query, -1);
}
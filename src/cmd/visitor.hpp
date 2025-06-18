#ifndef CAESAR_VISITOR_HPP
#define CAESAR_VISITOR_HPP

#define ABSTRACT_VISIT_METHOD_DECL(classname) \
  virtual void visit(classname*) = 0;

#define CONCRETE_VISIT_METHOD_DECL(classname) void visit(classname*);

#define ABSTRACT_ACCEPT_METHOD_DECL virtual void accept(Visitor* visitor) = 0;

#define CONCRETE_ACCEPT_METHOD_DECL void accept(Visitor* visitor);

#define CONCRETE_ACCEPT_METHOD_IMPL(classname) \
  void classname::accept(Visitor* visitor) { return visitor->visit(this); }

class BinaryExpr;
class NumLiteral;
class StrLiteral;

class Visitor {
 public:
  virtual ~Visitor() = default;

  ABSTRACT_VISIT_METHOD_DECL(BinaryExpr);
  ABSTRACT_VISIT_METHOD_DECL(NumLiteral);
};

#endif

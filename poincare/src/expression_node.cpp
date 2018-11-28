#include <poincare/expression_node.h>
#include <poincare/expression.h>
#include <poincare/addition.h>
#include <poincare/arc_tangent.h>
#include <poincare/division.h>
#include <poincare/power.h>
#include <poincare/rational.h>
#include <poincare/sign_function.h>
#include <poincare/square_root.h>
#include <poincare/subtraction.h>
#include <poincare/constant.h>
#include <poincare/undefined.h>

namespace Poincare {

Expression ExpressionNode::replaceSymbolWithExpression(const SymbolAbstract & symbol, const Expression & expression) {
  return Expression(this).defaultReplaceSymbolWithExpression(symbol, expression);
}

Expression ExpressionNode::replaceUnknown(const Symbol & symbol) {
  return Expression(this).defaultReplaceUnknown(symbol);
}

Expression ExpressionNode::setSign(Sign s, Context * context, Preferences::AngleUnit angleUnit) {
  assert(false);
  return Expression();
}

int ExpressionNode::polynomialDegree(Context & context, const char * symbolName) const {
  for (ExpressionNode * c : children()) {
    if (c->polynomialDegree(context, symbolName) != 0) {
      return -1;
    }
  }
  return 0;
}

int ExpressionNode::getPolynomialCoefficients(Context & context, const char * symbolName, Expression coefficients[]) const {
  return Expression(this).defaultGetPolynomialCoefficients(context, symbolName, coefficients);
}

Expression ExpressionNode::shallowReplaceReplaceableSymbols(Context & context) {
  return Expression(this).defaultReplaceReplaceableSymbols(context);
}

int ExpressionNode::getVariables(Context & context, isVariableTest isVariable, char * variables, int maxSizeVariable) const {
 int numberOfVariables = 0;
  for (ExpressionNode * c : children()) {
   int n = c->getVariables(context, isVariable, variables, maxSizeVariable);
   if (n < 0) {
     return n;
   }
   numberOfVariables = n > numberOfVariables ? n : numberOfVariables;
 }
 return numberOfVariables;
}

float ExpressionNode::characteristicXRange(Context & context, Preferences::AngleUnit angleUnit) const {
  /* A expression has a characteristic range if at least one of its childAtIndex has
   * one and the other are x-independant. We keep the biggest interesting range
   * among the childAtIndex interesting ranges. */
  float range = 0.0f;
  for (ExpressionNode * c : children()) {
    float opRange = c->characteristicXRange(context, angleUnit);
    if (std::isnan(opRange)) {
      return NAN;
    } else if (range < opRange) {
      range = opRange;
    }
  }
  return range;
}

Expression ExpressionNode::realPart(Context & context, Preferences::AngleUnit angleUnit) const {
  return Expression();
}

Expression ExpressionNode::imaginaryPart(Context & context, Preferences::AngleUnit angleUnit) const {
  return Expression();
}

Expression ExpressionNode::complexNorm(Context & context, Preferences::AngleUnit angleUnit) const {
  Expression a = realPart(context, angleUnit);
  Expression b = imaginaryPart(context, angleUnit);
  if (!a.isUninitialized() && !b.isUninitialized()) {
    // sqrt(a^2+b^2)
    return SquareRoot::Builder(
        Addition(
          Power(a, Rational(2)).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation),
          Power(b, Rational(2)).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation)
        ).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation)
      ).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation);
  }
  return Expression();
}

Expression ExpressionNode::complexArgument(Context & context, Preferences::AngleUnit angleUnit) const {
  Expression a = realPart(context, angleUnit);
  Expression b = imaginaryPart(context, angleUnit);
  if (!a.isUninitialized() && !b.isUninitialized()) {
    if (b.type() != Type::Rational || !static_cast<Rational &>(b).isZero()) {
      // arctan(a/b) or (180/Pi)*arctan(a/b)
      Expression arcTangent = ArcTangent::Builder(Division(a, b.clone()).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation)).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation);
      if (angleUnit == Preferences::AngleUnit::Degree) {
        arcTangent = arcTangent.degreeToRadian(context, angleUnit, ReductionTarget::BottomUpComputation);
      }
      // sign(b) * Pi/2 - arctan(a/b)
      return Subtraction(
          Multiplication(
            SignFunction::Builder(b).shallowReduce(context, angleUnit),
            Division(Constant(Ion::Charset::SmallPi), Rational(2)).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation)
          ),
          arcTangent
        ).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation);
    } else {
      // (1-sign(a))*Pi/2
      return Multiplication(
               Subtraction(
                 Rational(1),
                 SignFunction::Builder(a).shallowReduce(context, angleUnit)
               ).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation),
               Division(Constant(Ion::Charset::SmallPi), Rational(2)).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation)
            ).shallowReduce(context, angleUnit, ReductionTarget::BottomUpComputation);
    }
  }
  return Expression();
}

int ExpressionNode::SimplificationOrder(const ExpressionNode * e1, const ExpressionNode * e2, bool canBeInterrupted) {
  if (e1->type() > e2->type()) {
    if (canBeInterrupted && Expression::shouldStopProcessing()) {
      return 1;
    }
    return -(e2->simplificationOrderGreaterType(e1, canBeInterrupted));
  } else if (e1->type() == e2->type()) {
    return e1->simplificationOrderSameType(e2, canBeInterrupted);
  } else {
    if (canBeInterrupted && Expression::shouldStopProcessing()) {
      return -1;
    }
    return e1->simplificationOrderGreaterType(e2, canBeInterrupted);
  }
}

int ExpressionNode::simplificationOrderSameType(const ExpressionNode * e, bool canBeInterrupted) const {
  int index = 0;
  for (ExpressionNode * c : children()) {
    // The NULL node is the least node type.
    if (e->numberOfChildren() <= index) {
      return 1;
    }
    int childIOrder = SimplificationOrder(c, e->childAtIndex(index), canBeInterrupted);
    if (childIOrder != 0) {
      return childIOrder;
    }
    index++;
  }
  // The NULL node is the least node type.
  if (e->numberOfChildren() > numberOfChildren()) {
    return -1;
  }
  return 0;
}

void ExpressionNode::deepReduceChildren(Context & context, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target) {
  Expression(this).defaultDeepReduceChildren(context, angleUnit, target);
}

Expression ExpressionNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit, ReductionTarget target) {
  return Expression(this).defaultShallowReduce(context, angleUnit);
}

Expression ExpressionNode::shallowBeautify(Context & context, Preferences::AngleUnit angleUnit) {
  return Expression(this).defaultShallowBeautify(context, angleUnit);
}

bool ExpressionNode::isOfType(Type * types, int length) const {
  for (int i = 0; i < length; i++) {
    if (type() == types[i]) {
      return true;
    }
  }
  return false;
}

void ExpressionNode::setChildrenInPlace(Expression other) {
  Expression(this).defaultSetChildrenInPlace(other);
}

Expression ExpressionNode::denominator(Context & context, Preferences::AngleUnit angleUnit) const {
  return Expression();
}

}

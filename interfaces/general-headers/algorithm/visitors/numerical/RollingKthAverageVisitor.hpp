/*
  FILE: RollingKthAverageVisitor.hpp
  AUTHOR: Shane Neph & Scott Kuehn
  CREATE DATE: Mon Aug 20 14:22:38 PDT 2007
  PROJECT: windowing visitors
  ID: $Id$
*/


// Macro Guard
#ifndef ROLLING_KTH_VISITOR_AVERAGE_HPP
#define ROLLING_KTH_VISITOR_AVERAGE_HPP


// Files included
#include <cmath>
#include <cstdlib>
#include <set>
#include <string>

// Files included
#include "algorithm/visitors/helpers/ProcessVisitorRow.hpp"
#include "algorithm/visitors/numerical/RollingKthVisitor.hpp"
#include "data/measurement/AssayMeasurement.hpp"
#include "data/measurement/NaN.hpp"


namespace Visitors {

  template <
            typename Process,
            typename BaseVisitor,
            typename ExceptionType = Ext::ArgumentError
           >
  struct RollingKthAverage : RollingKth<Visitors::Helpers::DoNothing, BaseVisitor, ExceptionType> {

    typedef RollingKth<Visitors::Helpers::DoNothing, BaseVisitor, ExceptionType> BaseClass;
    typedef Process ProcessType;
    typedef typename BaseClass::T1 T1;
    typedef typename BaseClass::T2 T2;
    typedef T2* PtrType;

    //==============
    // Construction
    //==============
    explicit RollingKthAverage(double kth = 0.8, const ProcessType& pt = ProcessType())
        : BaseClass(kth, Visitors::Helpers::DoNothing()), currentAtPos_(0), pt_(pt) { /* */ }

    //====================================
    // Repositioning and Reporting Phases
    //====================================
    inline void DoneReference() {
      BaseClass::DoneReference();

      // The calculations below are based upon suggestions from wikipedia.
      //   They are different from the base class' implementation
      typedef typename Signal::AssayMeasurement<T2>::value_type VT;
      std::size_t size = BaseClass::scoresBuf_.size();
      std::size_t kthPosUp = static_cast<std::size_t>(std::ceil(static_cast<double>(BaseClass::kthValue_ * size)));
      std::size_t kthPosDown = static_cast<std::size_t>(std::floor(static_cast<double>(BaseClass::kthValue_ * size)));
      if ( kthPosUp > 0 ) // make zero-based
        --kthPosUp;
      if ( kthPosDown > 0 ) // make zero-based
        --kthPosDown;

      if ( size > 1 ) {
        if ( kthPosUp == kthPosDown ) { // a true integer; take average of two adjacent integers
          double one = **BaseClass::currentMarker_;
          typename BaseClass::ScoreTypeContainer::iterator next = BaseClass::currentMarker_;
          double two = **++next;
          pt_.operator()((one + two)/2.0);
        } else if ( BaseClass::currentAtPos_ == kthPosUp ) {
          pt_.operator()(static_cast<VT>(**BaseClass::currentMarker_));
        } else { // BaseClass::currentAtPos_ == kthPosDown; round up to kthPosUp per wikipedia
          typename BaseClass::ScoreTypeContainer::iterator next = BaseClass::currentMarker_;
          pt_.operator()(static_cast<VT>(**++next));
        }
      } else if ( 1 == size ) {
        pt_.operator()(static_cast<VT>(**BaseClass::currentMarker_));
      } else {
        static const Signal::NaN nan = Signal::NaN();
        pt_.operator()(nan);
      }
    }

    //===========
    // Cleanup()
    //===========
    virtual ~RollingKthAverage()
      { /* */ }

  protected:
    typedef Ordering::CompValueThenAddressLesser<T2, T2> Comp;
    typedef std::set<PtrType, Comp> ScoreTypeContainer;

  protected:
    double kthValue_;
    std::size_t currentAtPos_;
    ProcessType pt_;
    typename ScoreTypeContainer::iterator currentMarker_;
  };

} // namespace Visitors

#endif // ROLLING_KTH_VISITOR_AVERAGE_HPP
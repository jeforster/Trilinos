#pragma once
#ifndef __TEST_CHOL_TRI_SOLVE_BY_BLOCKS_HPP__
#define __TEST_CHOL_TRI_SOLVE_BY_BLOCKS_HPP__

#include "util.hpp"

#include "crs_matrix_base.hpp"
#include "crs_matrix_view.hpp"
#include "crs_row_view.hpp"

#include "dense_matrix_base.hpp"
#include "dense_matrix_view.hpp"

#include "graph_helper_scotch.hpp"
#include "crs_matrix_helper.hpp"

#include "team_view.hpp"
#include "task_view.hpp"

#include "parallel_for.hpp"

#include "team_factory.hpp"
#include "task_factory.hpp"
#include "task_team_factory.hpp"

#include "tri_solve.hpp"

#include "tmg_dense_matrix_base_simple.hpp"

namespace Tacho {

  using namespace std;

  template<typename ValueType,
           typename OrdinalType,
           typename SizeType = OrdinalType,
           typename SpaceType = void,
           typename MemoryTraits = void>
  KOKKOS_INLINE_FUNCTION
  int testCholTriSolveByBlocks(const string file_input,
                                const OrdinalType nb,
                                const OrdinalType nrhs) {
    typedef ValueType   value_type;
    typedef OrdinalType ordinal_type;
    typedef SizeType    size_type;

    typedef TaskTeamFactory<Kokkos::Experimental::TaskPolicy<SpaceType>,
      Kokkos::Experimental::Future<int,SpaceType>,
      Kokkos::Impl::TeamThreadRangeBoundariesStruct> TaskFactoryType;
    typedef ParallelFor ForType;

    typedef CrsMatrixBase<value_type,ordinal_type,size_type,SpaceType,MemoryTraits> CrsMatrixBaseType;
    typedef GraphHelper_Scotch<CrsMatrixBaseType> GraphHelperType;

    typedef CrsMatrixView<CrsMatrixBaseType> CrsMatrixViewType;
    typedef TaskView<CrsMatrixViewType,TaskFactoryType> CrsTaskViewType;
    
    typedef CrsMatrixBase<CrsTaskViewType,ordinal_type,size_type,SpaceType,MemoryTraits> CrsHierMatrixBaseType;

    typedef CrsMatrixView<CrsHierMatrixBaseType> CrsHierMatrixViewType;
    typedef TaskView<CrsHierMatrixViewType,TaskFactoryType> CrsHierTaskViewType;

    typedef DenseMatrixBase<value_type,ordinal_type,size_type,SpaceType,MemoryTraits> DenseMatrixBaseType;

    typedef DenseMatrixView<DenseMatrixBaseType> DenseMatrixViewType;
    typedef TaskView<DenseMatrixViewType,TaskFactoryType> DenseTaskViewType;

    typedef DenseMatrixBase<DenseTaskViewType,ordinal_type,size_type,SpaceType,MemoryTraits> DenseHierMatrixBaseType;

    typedef DenseMatrixView<DenseHierMatrixBaseType> DenseHierMatrixViewType;
    typedef TaskView<DenseHierMatrixViewType,TaskFactoryType> DenseHierTaskViewType;

    typedef Tmg_DenseMatrixBase_Simple<DenseMatrixBaseType> TmgType;

    int r_val = 0;

    __DOT_LINE__;
    cout << "testCholTriSolveByBlocks:: input = " << file_input
         << ", nb = " << nb 
         << ", nrhs = " << nrhs << endl;
    __DOT_LINE__;

    CrsMatrixBaseType AA("AA");
    {
      ifstream in;
      in.open(file_input);
      if (!in.good()) {
        cout << "Failed in open the file: " << file_input << endl;
        return ++r_val;
      }
      AA.importMatrixMarket(in);
    }

    CrsMatrixBaseType   UU_Unblocked("UU_Unblocked"), 
      /**/              UU_ByBlocks ("UU_ByBlocks");
    DenseMatrixBaseType BB_Unblocked("BB", AA.NumRows(), nrhs), 
      /**/              BB_ByBlocks ("CC", AA.NumRows(), nrhs);
    
    CrsHierMatrixBaseType   HU("HU");
    DenseHierMatrixBaseType HB("HB");
    {
      GraphHelperType S(AA);
      S.computeOrdering();

      CrsMatrixBaseType PA("Permuted AA");
      PA.copy(S.PermVector(), S.InvPermVector(), AA);

      UU_Unblocked.copy(Uplo::Upper, PA);
      UU_ByBlocks.copy(Uplo::Upper, PA);

      CrsMatrixHelper::flat2hier(Uplo::Upper, UU_ByBlocks, HU,
                                 S.NumBlocks(),
                                 S.RangeVector(),
                                 S.TreeVector());

      DenseMatrixHelper::flat2hier(BB_ByBlocks, HB,
                                   S.NumBlocks(),
                                   S.RangeVector(),
                                   nb);
    }

    TmgType tmg(AA.NumRows(), nrhs);

    cout << "testCholTriSolveByBlocks::Begin - " << r_val << endl;
    typename TaskFactoryType::policy_type policy;
    TaskFactoryType::setPolicy(&policy);

    {
      CrsHierTaskViewType TU(&HU);
      for (size_type k=0;k<HU.NumNonZeros();++k)
        HU.Value(k).fillRowViewArray();

      DenseHierTaskViewType TB(&HB);
      r_val += tmg.fill(BB_ByBlocks);
      
      auto future_factor 
        = TaskFactoryType::Policy().create_team(Chol<Uplo::Upper,AlgoChol::ByBlocks>::
                             TaskFunctor<ForType,CrsHierTaskViewType>(TU), 0);
      TaskFactoryType::Policy().spawn(future_factor);
      
      auto future_forward_solve 
        = TaskFactoryType::Policy().create_team(TriSolve<Uplo::Upper,Trans::ConjTranspose,AlgoTriSolve::ByBlocks>
                                                ::TaskFunctor<ForType,CrsHierTaskViewType,DenseHierTaskViewType>
                                                (Diag::NonUnit, TU, TB), 1);

      TaskFactoryType::Policy().add_dependence(future_forward_solve, future_factor);      
      TaskFactoryType::Policy().spawn(future_forward_solve);
      
      auto future_backward_solve 
        = TaskFactoryType::Policy().create_team(TriSolve<Uplo::Upper,Trans::NoTranspose,AlgoTriSolve::ByBlocks>
                                                ::TaskFunctor<ForType,CrsHierTaskViewType,DenseHierTaskViewType>
                                                (Diag::NonUnit, TU, TB), 1);

      TaskFactoryType::Policy().add_dependence(future_backward_solve, future_forward_solve);            
      TaskFactoryType::Policy().spawn(future_backward_solve);

      Kokkos::Experimental::wait(TaskFactoryType::Policy());
      cout << BB_ByBlocks << endl;
    }
    {
      CrsTaskViewType U(&UU_Unblocked);
      U.fillRowViewArray();

      DenseTaskViewType B(&BB_Unblocked);
      r_val += tmg.fill(BB_Unblocked);
      
      {
        auto future = TaskFactoryType::Policy().create_team(Chol<Uplo::Upper,AlgoChol::UnblockedOpt,Variant::One>
                                                            ::TaskFunctor<ForType,CrsTaskViewType>(U), 0);
        TaskFactoryType::Policy().spawn(future);
        Kokkos::Experimental::wait(TaskFactoryType::Policy());
      }
      {
        auto future = TaskFactoryType::Policy().create_team(TriSolve<Uplo::Upper,Trans::ConjTranspose,AlgoTriSolve::Unblocked>
                                                            ::TaskFunctor<ForType,CrsTaskViewType,DenseTaskViewType>
                                                            (Diag::NonUnit, U, B), 0);
        TaskFactoryType::Policy().spawn(future);
        Kokkos::Experimental::wait(TaskFactoryType::Policy());
      }
      {
        auto future = TaskFactoryType::Policy().create_team(TriSolve<Uplo::Upper,Trans::NoTranspose,AlgoTriSolve::Unblocked>
                                                            ::TaskFunctor<ForType,CrsTaskViewType,DenseTaskViewType>
                                                            (Diag::NonUnit, U, B), 0);
        
        TaskFactoryType::Policy().spawn(future);
        Kokkos::Experimental::wait(TaskFactoryType::Policy());
      }

      cout << BB_Unblocked << endl;
    }

    {
      const auto epsilon = sqrt(NumericTraits<value_type>::epsilon());
      for (ordinal_type j=0;j<BB_Unblocked.NumCols();++j)
        for (ordinal_type i=0;i<BB_Unblocked.NumRows();++i) {
          auto tmp = abs(BB_ByBlocks.Value(i, j) - BB_Unblocked.Value(i, j));          
          __ASSERT_TRUE__(tmp < epsilon);
        }
    }

    cout << "testCholTriSolveByBlocks::End - " << r_val << endl;    

    string eval;
    __EVAL_STRING__(r_val, eval);
    cout << "testCholTriSolveByBlocks::Eval - " << eval << endl;
    
    __DOT_LINE__;

    return r_val;
  }
}

#endif

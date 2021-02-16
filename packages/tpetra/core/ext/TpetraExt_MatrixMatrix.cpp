/*
// @HEADER
// ***********************************************************************
//
//          Tpetra: Templated Linear Algebra Services Package
//                 Copyright (2008) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
// @HEADER
*/

#include "TpetraExt_MatrixMatrix.hpp"

#ifdef HAVE_TPETRA_EXPLICIT_INSTANTIATION

#include "TpetraCore_ETIHelperMacros.h"
#include "TpetraExt_MatrixMatrix_def.hpp"

namespace Tpetra {

  TPETRA_ETI_MANGLING_TYPEDEFS()

#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS 
  TPETRA_INSTANTIATE_SLGN(TPETRA_MATRIXMATRIX_INSTANT)
#else
  TPETRA_INSTANTIATE_SN(TPETRA_MATRIXMATRIX_INSTANT)
#endif

  // Zoltan2 wants Scalar = int (Bug 6298).
  // This is already covered above for the GO = int case.
  //
  // FIXME (mfh 07 Oct 2015) I'm hand-rolling this for now.
  // It would be better to let CMake do the ETI generation,
  // as with CrsMatrix.

#ifdef HAVE_TPETRA_INST_INT_LONG
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_LONG( NT ) \
  TPETRA_MATRIXMATRIX_INSTANT( int, int, long, NT )

  TPETRA_INSTANTIATE_N(TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_LONG)
#else
#define TPETRA_MATRIXMATRIX_INSTANT_SC_INT( NT ) \
  TPETRA_MATRIXMATRIX_INSTANT( int, NT )

  TPETRA_INSTANTIATE_N(TPETRA_MATRIXMATRIX_INSTANT_SC_INT)
#endif

#endif // HAVE_TPETRA_INST_INT_LONG

#ifdef HAVE_TPETRA_INST_INT_LONG_LONG
#ifdef TPETRA_ENABLE_TEMPLATE_ORDINALS
#define TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_LONG_LONG( NT ) \
  TPETRA_MATRIXMATRIX_INSTANT( int, int, longlong, NT )

  TPETRA_INSTANTIATE_N(TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_LONG_LONG)
#else
#define TPETRA_MATRIXMATRIX_INSTANT_SC_INT( NT ) \
  TPETRA_MATRIXMATRIX_INSTANT( int, NT )

  TPETRA_INSTANTIATE_N(TPETRA_MATRIXMATRIX_INSTANT_SC_INT)
#endif

#endif // HAVE_TPETRA_INST_INT_LONG_LONG

#ifdef HAVE_TPETRA_INST_INT_UNSIGNED
#define TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_UNSIGNED( NT ) \
  TPETRA_MATRIXMATRIX_INSTANT( int, int, unsigned, NT )

  TPETRA_INSTANTIATE_N(TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_UNSIGNED)

#endif // HAVE_TPETRA_INST_INT_UNSIGNED

#ifdef HAVE_TPETRA_INST_INT_UNSIGNED_LONG
#define TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_UNSIGNED_LONG( NT ) \
  TPETRA_MATRIXMATRIX_INSTANT( int, int, unsignedlong, NT )

  TPETRA_INSTANTIATE_N(TPETRA_MATRIXMATRIX_INSTANT_SC_INT_LO_INT_GO_UNSIGNED_LONG)

#endif // HAVE_TPETRA_INST_INT_UNSIGNED_LONG

} // namespace Tpetra

#endif // HAVE_TPETRA_EXPLICIT_INSTANTIATION

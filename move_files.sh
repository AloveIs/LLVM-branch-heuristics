base=..

echo "Copying header file..."
cp BranchProbabilityInfo.h $base/llvm/include/llvm/Analysis/BranchProbabilityInfo.h \
	&& echo "Done." || echo "Failed copying header file."
echo "Copying source file..."
cp BranchProbabilityInfo.cpp $base/llvm/lib/Analysis/BranchProbabilityInfo.cpp \
	&& echo "Done." || echo "Failed copying source file."

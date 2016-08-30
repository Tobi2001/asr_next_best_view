/**

Copyright (c) 2016, Allgeyer Tobias, Aumann Florian, Borella Jocelyn, Braun Kai, Heller Florian, Hutmacher Robin, Karrenbauer Oliver, Marek Felix, Mayr Matthias, Mehlhaus Jonas, Meißner Pascal, Schleicher Ralf, Stöckle Patrick, Stroh Daniel, Trautmann Jeremias, Walter Milena
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. The use is explicitly not permitted to any application which deliberately try to kill or do harm to any living creature.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "next_best_view/rating/impl/DefaultScoreContainer.hpp"
#include <ostream>

namespace next_best_view {
    DefaultScoreContainer::DefaultScoreContainer() : BaseScoreContainer(), mMovementCostsBaseTranslation(0.0),
                                                                            mMovementCostsBaseRotation(0.0),
                                                                            mMovementCostsPTU(0.0),
                                                                            mRecognitionCosts(0.0) {
	}

	DefaultScoreContainer::~DefaultScoreContainer() { }

    std::ostream& operator<<(std::ostream &strm, const next_best_view::DefaultScoreContainer &score) {
        using std::endl;
        strm << "UnweightedInverseMovementCostsBaseTranslation: " << score.getUnweightedInverseMovementCostsBaseTranslation() << endl
             << "unweightedInverseMovementCostsBaseRotation: " << score.getUnweightedInverseMovementCostsBaseRotation() << endl
             << "unweightedInverseMovementCostsPTU: " << score.getUnweightedInverseMovementCostsPTU() << endl
             << "unweightedInverseRecognitionCosts: " << score.getUnweightedInverseRecognitionCosts() << endl
             << "utilityNormalization: " << score.getUtilityNormalization() << endl
             << "weightedUnnormalizedUtility: " << score.getWeightedUnnormalizedUtility() << endl
             << "weightedNormalizedUtility: " << score.getWeightedNormalizedUtility() << endl
             << "weightedInverseCosts: " << score.getWeightedInverseCosts() << endl
             << "objectUtility: {" << endl;
        auto ratedObjects = score.getObjectTypes();
        for (auto objType : *ratedObjects) {
            strm << *objType << " -> " << score.getUnweightedUnnormalizedObjectUtility(*objType) << endl;
        }
        return strm << "}" << endl;
    }

    std::ostream& operator<<(std::ostream &strm, const next_best_view::DefaultScoreContainerPtr &score) {
        return strm << *score;
    }
}

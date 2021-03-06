/**

Copyright (c) 2016, Aumann Florian, Borella Jocelyn, Heller Florian, Meißner Pascal, Schleicher Ralf, Stöckle Patrick, Stroh Daniel, Trautmann Jeremias, Walter Milena, Wittenbeck Valerij
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. The use is explicitly not permitted to any application which deliberately try to kill or do harm to any living creature.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once

#include "next_best_view/common/GeneralFilter.hpp"
#include "next_best_view/crop_box/CropBoxWrapper.hpp"
#include "pcl-1.7/pcl/filters/impl/crop_box.hpp"

namespace next_best_view {

    /*!
     * \brief The CropBoxFilter class, used to filter objects in crop boxes.
     */
    class CropBoxFilter : public GeneralFilter {
    private:
        boost::shared_ptr<std::vector<CropBoxWrapperPtr>> mCropBoxPtrList;

    public:
        /*!
         * \brief CropBoxFilter
         * \param xml_path path to a xml file containing crop box information (name, poisition, size).
         */
        CropBoxFilter(const std::string &xml_path);

        /*!
         * \brief doFiltering Does the actual filtering. setInputCloud(...) must be called before.
         * \param indicesPtr the filtered objects indices.
         */
        void doFiltering(IndicesPtr &indicesPtr);

        /*!
         * \brief getCropBoxPtrList
         * \return returns mCropBoxPtrList for visualization.
         */
        boost::shared_ptr<std::vector<CropBoxWrapperPtr>> getCropBoxWrapperPtrList();
    };

    /*!
     * \brief CropBoxFilterPtr typedef for consistency/less code to create boost::shared_ptr to CropBoxFilter.
     */
    typedef boost::shared_ptr<CropBoxFilter> CropBoxFilterPtr;
}

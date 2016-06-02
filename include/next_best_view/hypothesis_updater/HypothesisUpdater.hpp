/*
 * HypothesisUpdater.hpp
 *
 *  Created on: Oct 2, 2014
 *      Author: ralfschleicher
 */

#ifndef HYPOTHESISUPDATER_HPP_
#define HYPOTHESISUPDATER_HPP_

#include "next_best_view/common/CommonClass.hpp"
#include "typedef.hpp"

namespace next_best_view {
	/*!
	 * \brief HypothesisUpdater is an abstract class for updating the objects in the point cloud.
	 */
	class HypothesisUpdater : public CommonClass {
	public:
		HypothesisUpdater();
		virtual ~HypothesisUpdater() = 0;

        /*!
         * \brief Updates the point cloud under the assumption that the given viewport was chosen.
         * \param objectTypeSetPtr the object type names that shall be updated.
         * \param viewportPoint the viewport that was chosen
         * \return the number of deactivated normals
         */
        virtual unsigned int update(const ObjectTypeSetPtr &objectTypeSetPtr, const ViewportPoint &viewportPoint) = 0;
	};

	/*!
	 * \brief Definition for the shared pointer type of the class.
	 */
	typedef boost::shared_ptr<HypothesisUpdater> HypothesisUpdaterPtr;
}

#endif /* HYPOTHESISUPDATER_HPP_ */

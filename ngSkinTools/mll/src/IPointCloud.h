#pragma once
#include "maya.h"




/**
 * Interface for point cloud implementation
 * intended usage of this interface:
 *    1. cloud is populated with data, first calling setCapacity() to inform cloud, how many
 *       points is incomming, then addPoint() calls fill up data array
 *    2. cloud is queried with getNearbyVerts() calls, one by one.
 *       queuing multiple iterators at once should not be needed.
 */
class IPointCloud
{
    public:
		/**
		 * iterator interface that must be implemented for IPointCloud::getNearbyVerts
		 */
		class IPointIterator {
		public:
			/**
			 * returns vertex data, supplied along the point in IPointCloud::addPoint
			 */
			virtual void * getData() const =0;

			/**
			 * returns distance to this point from sample point given to IPointCloud::getNearbyVerts
			 */
			virtual double getDistance() const =0;

			/**
			 * iterate to next point; MUST be called before accessing first point, too.
			 */
			virtual bool next()=0;
			virtual ~IPointIterator(){};
		};

		virtual ~IPointCloud(){};


		/**
		 * calling threads will use this method to tell cloud
		 * how many points there might be. memory allocation
		 * will be much faster than adding points one by one.
		 */
		virtual void setCapacity(const unsigned int capacity) = 0;

		/**
		 * add point to point cloud
		 */
        virtual void addPoint(void * vertData, const MPoint position) = 0;


		/**
		 * initialize iteration for nearby vertices
		 * point cloud is responsible itself for managing iterator instance
		 */
		virtual IPointIterator * getNearbyVerts(const double radius, const MPoint position) = 0;
};

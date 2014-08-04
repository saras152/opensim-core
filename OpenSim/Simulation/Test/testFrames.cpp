/* -------------------------------------------------------------------------- *
 *                          OpenSim:  testFrames.cpp                          *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2012 Stanford University and the Authors                *
 * Author(s): Ajay Seth                                                       *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

//==============================================================================
//
//	Tests Include:
//      1. BodyFrame
//		2. FixedFrame
//		
//     Add tests here as Frames are added to OpenSim
//
//==============================================================================
#include <ctime>  // clock(), clock_t, CLOCKS_PER_SEC
#include <OpenSim/Simulation/osimSimulation.h>
#include <OpenSim/Analyses/osimAnalyses.h>
#include <OpenSim/Auxiliary/auxiliaryTestFunctions.h>

using namespace OpenSim;
using namespace std;

void testBodyFrame();
void testFixedFrameOnBodyFrame();

int main()
{
	SimTK::Array_<std::string> failures;

	try { testBodyFrame(); }
    catch (const std::exception& e){
		cout << e.what() <<endl; failures.push_back("testBodyFrame");
	}
		
	try { testFixedFrameOnBodyFrame(); }
    catch (const std::exception& e){
		cout << e.what() <<endl; failures.push_back("testFixedFrameOnBodyFrame");
	}

    if (!failures.empty()) {
        cout << "Done, with failure(s): " << failures << endl;
        return 1;
    }

	cout << "Done. All cases passed." << endl;

    return 0;
}

//==============================================================================
// Test Cases
//==============================================================================

void testBodyFrame()
{
	Model dPendulum("double_pendulum.osim");
	const OpenSim::Body& rod1 = dPendulum.getBodySet().get("rod1");
	BodyFrame rod1Frame(rod1);
	dPendulum.addModelComponent(&rod1Frame);
	SimTK::State& st = dPendulum.initSystem();
	for (double ang = 0; ang <= 90.0; ang += 10.){
		double radAngle = SimTK::convertDegreesToRadians(ang);
		const Coordinate& coord = dPendulum.getCoordinateSet().get("q1");
		coord.setValue(st, radAngle);
		SimTK::Transform xform = rod1Frame.calcTransformToGround(st);
		// By construction the transform should gove a translation of .353553, .353553, 0.0 since 0.353553 = .5 /sqr(2)
		double dNorm = (xform.p() - SimTK::Vec3(0.5*std::sin(radAngle), -0.5*std::cos(radAngle), 0.)).norm();
		assert(dNorm < 1e-6);
		// The rotation part is a pure bodyfixed Z-rotation by radAngle.
		SimTK::Vec3 angles = xform.R().convertRotationToBodyFixedXYZ();
		assert(std::fabs(angles[0]) < 1e-6);
		assert(std::fabs(angles[1]) < 1e-6);
		assert(std::fabs(angles[2] - radAngle) < 1e-6);
	}
	dPendulum.disownAllComponents(); // BAD but avoids crash on Model going out of scope since rod1Frame is on stack!
	return;
}

void testFixedFrameOnBodyFrame()
{
	Model dPendulum("double_pendulum.osim");
	const OpenSim::Body& rod1 = dPendulum.getBodySet().get("rod1");
	BodyFrame rod1Frame(rod1);
	dPendulum.addModelComponent(&rod1Frame);
	FixedFrame atOriginFrame(rod1Frame);
	SimTK::Transform relXform;
	relXform.setP(SimTK::Vec3(0.0, .5, 0.0));
	relXform.updR().setRotationFromAngleAboutAxis(SimTK::Pi / 4.0, SimTK::CoordinateAxis(2));
	atOriginFrame.setTransform(relXform);
	dPendulum.addModelComponent(&atOriginFrame);
	SimTK::State& st = dPendulum.initSystem();
	const SimTK::Transform rod1FrameXform = rod1Frame.calcTransformToGround(st);
	SimTK::Transform xform = atOriginFrame.calcTransformToGround(st);
	// xform should have 0.0 translation
	assert(xform.p().norm() < 1e-6);
	dPendulum.disownAllComponents(); // BAD but avoids crash on Model going out of scope since frames are allocated on stack!
	return;
}




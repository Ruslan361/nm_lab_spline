#include "Problems.h"

Problem::Problem()
{
}

Problem::Problem(MODE _mode)
{
	mode = _mode;
	switch (mode)
	{
	case TEST:
		a = -1.0;
		b = 1.0;
		mu1 = 0.0;
		mu2 = 0.0;
		break;
	case MAIN1:
		a = 0.0;
		b = 3.14159265358979323846;
		mu1 = 0.0;
		mu2 = 0.0;
		break;
	case Main2:
		a = 0.0;
		b = 1.0;
		mu1 = 0.0;
		mu2 = 0.0;
		break;
	case Main3:
		a = 1.0;
		b = 3.0;
		mu1 = 0.0;
		mu2 = 0.0;
		break;
	case OSC:
		a = 0.0;
		b = 3.14159265358979323846;
		mu1 = 0.0;
		mu2 = 0.0;
		break;
	default:
		break;
	}
}

void Problem::operator=(const Problem& _problem)
{
	mode = _problem.mode;
	a = _problem.a;
	b = _problem.b;
	mu1 = _problem.mu1;
	mu2 = _problem.mu2;
}

double Problem::f(double x)
{
	switch (mode)
	{
	case TEST:
		return TestFunc(x);
	case MAIN1:
		return MainFunc1(x);
	case Main2:
		return MainFunc2(x);
	case Main3:
		return MainFunc3(x);
	case OSC:
		return OscFunc(x);
	default:
		break;
	}
}

double Problem::df(double x)
{
	switch (mode)
	{
	case TEST:
		return dTF(x);
	case MAIN1:
		return dF1(x);
	case Main2:
		return dF2(x);
	case Main3:
		return dF3(x);
	case OSC:
		return dFosc(x);
	default:
		break;
	}
}

double Problem::d2f(double x)
{
	switch (mode)
	{
	case TEST:
		return d2TF(x);
	case MAIN1:
		return d2F1(x);
	case Main2:
		return d2F2(x);
	case Main3:
		return d2F3(x);
	case OSC:
		return d2Fosc(x);
	default:
		break;
	}
}

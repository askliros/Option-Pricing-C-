#include "EarlyExcerciseRateVolPathDependent.h"
#include <cstdlib>
#include <ctime>
using namespace std;
#include<vector>

#include "Regression.h"

EarlyExcerciseRateVolPathDependent::EarlyExcerciseRateVolPathDependent(const Simulation& theSimulation, const Vanilla& theVanilla)
{
	this->simulationptr=theSimulation.clone();
	this->theVanillaPtr=theVanilla.clone();
	
	
}



vector<double> EarlyExcerciseRateVolPathDependent::Results() const
{
	//We value with PathDependents in case we have path dependency. If we are path independent, then PathDependents will have the same inputs as the StockPaths.
	Regression theregression(0.0);
	const Doub TOL=1.e-12;
	vector<double> Results(2,1);
	this->simulationptr->operator()();
	vector<vector<double>> StockPaths=this->simulationptr->getStockPaths();
	vector<vector<double>> RatePaths=this->simulationptr->getRatesPaths();
	vector<vector<double>> VolPaths=this->simulationptr->getVolatilityPaths();
	vector<vector<double>>PathDependents=this->simulationptr->getPathDependents();
	double IndicatorOfAsian=this->simulationptr->getIndicatorAsian();
	vector<double> FinalValues(StockPaths.size(),1);
	double Average=0.0;
	double AverageSquare=0.0;
	vector<double> Y;//The the intrinsic payoffs
	
	vector<double> X;//The value of the Stock
	vector<double> X1; //The value of the rates
	vector<double> X2; //The value of the volatility
	vector<double> X3; // The value of the Path Dependency
	VecDoub Xnr,Ynr,SIC,A;
	MatDoub XRatesVolStockPathDependent;
	double temporary;
	vector<int> MoneynessIndex;
	int sumfwd;

	
	//Evaluating the Payoff of the American Option using the Longstaff-Schwarz algorithm

	int sumfwdtimes=0;
	for(int i=0;i<PathDependents.size();i++)
	{
		PathDependents[i][PathDependents[0].size()-1]=theVanillaPtr->VanillaPayoff(PathDependents[i][PathDependents[0].size()-1]);
		//cout<<"The terminal value of the path is: "<<Paths[i][Paths[0].size()-1][0]<<endl;
				
	}
	
	
	
	
	for(int time=PathDependents[0].size()-2;time>0;time--)//for every time instant
	{
					//Finding the X variable and the index (where the payoff of the current time is nonzero)
					for(int pathindex=0;pathindex<PathDependents.size();pathindex++)//for every path
					{				
						if(theVanillaPtr->VanillaPayoff(PathDependents[pathindex][time])>0)
						{
							X.push_back(StockPaths[pathindex][time]);//Creates X StockPaths
							X1.push_back(RatePaths[pathindex][time]);//Creates X1 rates
							X2.push_back(sqrt(VolPaths[pathindex][time]));//Creates X2 of volatility
							X3.push_back(PathDependents[pathindex][time]);//Creates X3 of path dependency
							MoneynessIndex.push_back(pathindex);
							//creating Y
							temporary=0; //finding the temporary
							for(int ftime=time+1;ftime<PathDependents[0].size();ftime++)
							{
								sumfwd=0;
								if(PathDependents[pathindex][ftime]>0)
								{
                                    temporary=PathDependents[pathindex][ftime]*RatePaths[pathindex][ftime]/RatePaths[pathindex][time];
									sumfwd+=1;
									if(sumfwd>1)
										throw("More than one non zero payoffs!");
								}

							}
							Y.push_back(temporary); //creating the Y
							//StockPaths[pathindex][time]=theVanillaPtr->VanillaPayoff(StockPaths[pathindex][time]);
						}else
						{
							PathDependents[pathindex][time]=0.0; //making zero the values at that time instant where the payoffs are not positive
						}
					}

					


					if(X.size()!=Y.size())
					{
						cout<<"This happens at time: "<<time<<endl;
						throw("The size of X is not equal to the size of Y!");
						throw("something is wrong!");
						break;
					}

				//Preparing for the regression
                double var=0;

				Xnr.resize(X.size());
				Ynr.resize(Y.size());
				SIC.resize(X.size());
				A.resize(20);
				XRatesVolStockPathDependent.resize(X.size(),4); //Change dimension
				if(Xnr.size()==0)
				{
				   for(int indexi=0;indexi<20;indexi++)
					   A[indexi]=0.0;
				}
				else{
					cout<<PathDependents[MoneynessIndex[1]][time];

				
					for(int i=0;i<Y.size();i++)
					{
                      XRatesVolStockPathDependent[i][0]=X[i];//MatDoub assigning the StockValues
					  XRatesVolStockPathDependent[i][1]=X1[i];//MatDoub assigning the RateValues
					  XRatesVolStockPathDependent[i][2]=X2[i];//MatDoub assigning the VolatilityValues
					  XRatesVolStockPathDependent[i][3]=X3[i];//MatDoub assigning the Path Dependency
					}
				

				    A=theregression.regressratevolPathDependent(XRatesVolStockPathDependent,Ynr);
				}
				    

					   for(int i=0;i<Xnr.size();i++)
					   {
						   //When it is optimal to exercise( if the intrinsic value is greater than the expected value
						   double intrinsicvalue=theVanillaPtr->VanillaPayoff(PathDependents[MoneynessIndex[i]][time]);
						   // need to amend for the case of stock apths as well.
						   double expectedvalue=0.0;


						   //Change this
 expectedvalue=A[0]+A[1]*StockPaths[MoneynessIndex[i]][time]+A[2]*RatePaths[MoneynessIndex[i]][time]+A[3]*VolPaths[MoneynessIndex[i]][time]+A[4]*PathDependents[MoneynessIndex[i]][time];
 expectedvalue+=A[5]*StockPaths[MoneynessIndex[i]][time]*StockPaths[MoneynessIndex[i]][time]+A[6]*RatePaths[MoneynessIndex[i]][time]*RatePaths[MoneynessIndex[i]][time];
 expectedvalue+=A[7]*VolPaths[MoneynessIndex[i]][time]*VolPaths[MoneynessIndex[i]][time]+A[8]*PathDependents[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time]+A[9]*StockPaths[MoneynessIndex[i]][time]*RatePaths[MoneynessIndex[i]][time];
 expectedvalue+=A[10]*VolPaths[MoneynessIndex[i]][time]*StockPaths[MoneynessIndex[i]][time]+A[11]*StockPaths[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time];
 expectedvalue+=A[12]*RatePaths[MoneynessIndex[i]][time]*VolPaths[MoneynessIndex[i]][time]+A[13]*RatePaths[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time];
 expectedvalue+=A[14]*VolPaths[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time];
 expectedvalue+=A[15]*VolPaths[MoneynessIndex[i]][time]*StockPaths[MoneynessIndex[i]][time]*RatePaths[MoneynessIndex[i]][time];	
 expectedvalue+=A[16]*VolPaths[MoneynessIndex[i]][time]*StockPaths[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time];
 expectedvalue+=A[17]*PathDependents[MoneynessIndex[i]][time]*StockPaths[MoneynessIndex[i]][time]*RatePaths[MoneynessIndex[i]][time];	
 expectedvalue+=A[18]*VolPaths[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time]*RatePaths[MoneynessIndex[i]][time];	
 expectedvalue+=A[19]*VolPaths[MoneynessIndex[i]][time]*StockPaths[MoneynessIndex[i]][time]*RatePaths[MoneynessIndex[i]][time]*PathDependents[MoneynessIndex[i]][time];	
							 
							 
							 if(intrinsicvalue>expectedvalue)
						   {
							   for(int forwardtime=time+1;forwardtime<PathDependents[0].size();forwardtime++)
								{
                                    PathDependents[MoneynessIndex[i]][forwardtime]=0;

								}
							    PathDependents[MoneynessIndex[i]][time]=theVanillaPtr->VanillaPayoff(PathDependents[MoneynessIndex[i]][time]);

						   }
						   else
						   {
							   PathDependents[MoneynessIndex[i]][time]=0.0;//Setting zero again when there is continuation
						   }
					   }
					  
					   Xnr.resize(0);
					   Ynr.resize(0);
					   SIC.resize(0);
					   A.resize(0);
					   X.erase(X.begin(),X.end());
					   Y.erase(Y.begin(),Y.end());
					   X1.erase(X1.begin(),X1.end());
					   XRatesVolStockPathDependent.resize(0,0);
					   MoneynessIndex.erase(MoneynessIndex.begin(),MoneynessIndex.end());
					   
	}
	double summ=0;
	for(int i=0;i<PathDependents.size();i++)
	{
		for(int j=1;j<PathDependents[0].size();j++)
		{
			summ+=PathDependents[i][j];
			PathDependents[i][j]=PathDependents[i][j]*RatePaths[i][j];
			Average+=PathDependents[i][j]/(static_cast<double>(PathDependents.size()));
			AverageSquare+=PathDependents[i][j]*PathDependents[i][j]/(static_cast<double>(PathDependents.size()));
			
		}
	}
	double value=summ/(static_cast<double>(PathDependents.size()));
	Results[0]=Average;
	Results[1]=sqrt(AverageSquare-Average*Average)/(sqrt(static_cast<double>(PathDependents.size())));
	return Results;
}


 Excercise* EarlyExcerciseRateVolPathDependent::clone() const
{
	return new EarlyExcerciseRateVolPathDependent(*this);
 }
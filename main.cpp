#include "solver.h"
#include "Functions.h"
#include <iostream>
#include <iomanip>
/*
�������:

Solver Sol(num_intervals, TEST); - ������ ������� (����� ���������� �����, ��� ������)
    ���� �����:
    TEST - ��������
    MAIN1 - 1 ������� (�������� � ����� �� ���� ����� ��������)
    MAIN2 - 2 �������
    MAIN3 - 3 �������
    OSC - ������������� 

����� ������� ����� ��� ����������
    1 ������� ����������� �� num_nodes ��� ��� ��� ����� �������:
    getA()
    getB()
    getC()
    getD() - ��� �� ��� ���������� ������������� (���������� ������)
    getX_for_coef_table() - ������������ ���� (������)
    get_n_step() - ���������� ���, � ������� ��� �� �������� �����(�� , ����� � �� �����)

    2 � 3 ������� : (����������� �� num_nodes*2 ��� ��� ��� ����������� �����)
    getX() - ������������ ���� (������)
    getF()
    getDF()
    getD2F()
    getS()
    getDS()
    getD2S() - ��� �� ������� �������� ������� � ������� ������� F - �������� S - �������,
               D � D2 - �� �����������
    get_N_step() - ���������� ���, � ������� ��� �� �������� �����(�� , ����� � �� �����)

    �������:
    getF_ERRROR();
    getF_ERRROR_X();
    getDF_ERRROR();
    getDF_ERRROR_X();
    getD2F_ERRROR();
    getD2F_ERRROR_X(); - ��� ���������� ������������ ����������� � �������������� �� x;

*/

int main() {
   
    int num_intervals{ 3 };
    Solver Sol(num_intervals, MAIN1, EGU);
    Sol.Solve();
    
    auto f = Sol.getF();
    auto df = Sol.getDF();
    auto d2f = Sol.getD2F();
    auto s = Sol.getS();
    auto ds = Sol.getDS();
    auto d2s = Sol.getD2S();
    //���� ������� ��� �������� ��� ������ ������

    
    for (int i = 0; i < s.size(); ++i) {
        if (i != s.size() - 1) {
            std::cout << std::setprecision(8) << s[i] << std::setw(15);
        }
        else {
            std::cout << std::setprecision(8) << s[i];
        }
    }
    std::cout << std::endl;

    for (int i = 0; i<ds.size(); ++i) {
        if (i != ds.size() - 1) {
            std::cout << std::setprecision(7)  << ds[i] << std::setw(15);
        }
        else {
            std::cout << std::setprecision(7) << ds[i];
        }
    }
    std::cout << std::endl;

    for (int i = 0; i < d2s.size(); ++i) {
        if (i != d2s.size() - 1) {
            std::cout << std::setprecision(6) << d2s[i] << std::setw(15);
        }
        else {
            std::cout << std::setprecision(6) << d2s[i];
        }
    }
    std::cout << std::endl;


    std::cout<<std::endl;

    for (int i = 0; i < f.size(); ++i) {
        if (i != f.size() - 1) {
            std::cout << std::setprecision(8) << f[i] << std::setw(15);
        }
        else {
            std::cout << std::setprecision(8) << f[i];
        }
    }
    std::cout << std::endl;

    for (int i = 0; i < df.size(); ++i) {
        if (i != df.size() - 1) {
            std::cout << std::setprecision(7) << df[i] << std::setw(15);
        }
        else {
            std::cout << std::setprecision(7) << df[i];
        }
    }
    std::cout << std::endl;

    for (int i = 0; i < d2f.size(); ++i) {
        if (i != d2f.size() - 1) {
            std::cout << std::setprecision(6) << d2f[i] << std::setw(15);
        }
        else {
            std::cout << std::setprecision(6) << d2f[i];
        }
    }
    std::cout << std::endl;

    return 0;
}
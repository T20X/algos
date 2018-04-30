#pragma once

#include <vector>
#include <iostream>
#include <iomanip>



#include <type_traits>



using namespace std;

struct IntMatrix
{
    explicit IntMatrix(int rows, int columns, int defaultVal = 0)
        : rowNum(rows), colNum(columns)
    {
        data.resize(rows);
        for (auto& rows : data)        
            rows.resize(columns, defaultVal);
    }

    void populateRow(int row, const std::initializer_list<int>&& values)
    {        
        data.at(row).insert(data[row].begin(), values.begin(), values.end());
    }

    int rowNum;
    int colNum;

    typedef vector<vector<int>> RepresentationType;
    RepresentationType data;
};

inline IntMatrix operator*(const IntMatrix& left, const IntMatrix& right)
{
    if (left.colNum != right.rowNum)
        throw std::runtime_error("left column not equal to right rows");

    IntMatrix r(left.rowNum, right.colNum);

    for (int i = 0; i < left.rowNum; i++)
    {
        for (int j = 0; j < right.colNum; j++)
        {
            int product = 0;
            for (int k = 0; k < right.rowNum; k++)            
                product += left.data[i][k] * right.data[k][j];

            r.data[i][j] = product;
            
        }
    }

    return r;
}

inline void operator<< (ostream& o, const IntMatrix& takmeout)
{
    for (int row = 0; row < takmeout.rowNum; row++)
    {
        cout << endl;
        for (int column = 0; column < takmeout.colNum; column++)
            cout << setw(3) << takmeout.data[row][column];
    }   
}

void matrixTest()
{
    IntMatrix m1(2, 3);
    IntMatrix m2(3, 2);

    m1.populateRow(0,{ 2,4,8 });
    m1.populateRow(1,{ 2,4,8 });

    m2.populateRow(0, { 2,4 });
    m2.populateRow(1, { 2,4 });
    m2.populateRow(2, { 2,4 });

    cout << m1;
    cout << "\n\n\n\n" << endl;

    cout << m2;
    cout << "\n\n\n\n" << endl;

    cout << m1 * m2;
    cout << "\n\n\n\n" << endl;

    cout << m2 * m1;
    cout << "\n\n\n\n" << endl;

    using int_zero = std::integral_constant<int, 0>;

    int i = 0;
    if (i == int_zero::value)
        i = 12;    
}

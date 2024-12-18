#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

vector<int> calculate_crc(const vector<int> &input_data, const vector<int> &crc_generator)
{
    int input_size = input_data.size();
    int generator_size = crc_generator.size();

    vector<int> padded_data = input_data;
    padded_data.resize(input_size + generator_size - 1, 0);

    for (int i = 0; i < input_size; ++i)
    {
        if (padded_data[i] == 1)
        {
            for (int j = 0; j < generator_size; ++j)
            {
                padded_data[i + j] ^= crc_generator[j];
            }
        }
    }

    vector<int> crc_result(generator_size - 1);
    copy(padded_data.begin() + input_size, padded_data.end(), crc_result.begin());
    return crc_result;
}

bool check_for_errors(const vector<int> &data_with_crc, const vector<int> &crc_generator)
{
    vector<int> crc_result = calculate_crc(data_with_crc, crc_generator);
    return all_of(crc_result.begin(), crc_result.end(), [](int bit)
                  { return bit == 0; });
}

void print_vector(const vector<int> &vec, const string &label)
{
    cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        cout << vec[i];
        if (i < vec.size() - 1)
        {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

void display_results(const vector<int> &data, const vector<int> &crc, const vector<int> &data_with_crc, bool no_errors)
{
    cout << "---- Results ----" << endl;
    print_vector(data, "Original Data");
    print_vector(crc, "CRC");
    print_vector(data_with_crc, "Data with CRC");
    cout << (no_errors ? "No errors detected in the data." : "Errors detected in the data!") << endl;
}

int main()
{
    vector<int> crc_generator = {1, 0, 1, 1, 1, 1, 1, 1}; // CRC generator polynomial
    random_device rd;                                     // For random number generation
    mt19937 rng(rd());                                    // Random number generator
    uniform_int_distribution<> bit_distribution(0, 1);    // Distribution for bits

    int data_size_1 = 21;
    int detected_errors = 0;
    int missed_errors = 0;
    vector<int> data_sample_1(data_size_1);
    generate(data_sample_1.begin(), data_sample_1.end(), [&]()
             { return bit_distribution(rng); });

    vector<int> crc_sample_1 = calculate_crc(data_sample_1, crc_generator);
    vector<int> data_with_crc_1 = data_sample_1;
    data_with_crc_1.insert(data_with_crc_1.end(), crc_sample_1.begin(), crc_sample_1.end());
    
    display_results(data_sample_1, crc_sample_1, data_with_crc_1, check_for_errors(data_with_crc_1, crc_generator));

    int data_size_2 = 250;
    vector<int> data_sample_2(data_size_2);
    generate(data_sample_2.begin(), data_sample_2.end(), [&]()
             { return bit_distribution(rng); });

    vector<int> crc_sample_2 = calculate_crc(data_sample_2, crc_generator);
    vector<int> data_with_crc_2 = data_sample_2;
    data_with_crc_2.insert(data_with_crc_2.end(), crc_sample_2.begin(), crc_sample_2.end());

    cout << "\nData Size = " << data_size_2 << endl;
    display_results(data_sample_2, crc_sample_2, data_with_crc_2, check_for_errors(data_with_crc_2, crc_generator));

    detected_errors = 0;
    missed_errors = 0;

    for (int i = 0; i < data_with_crc_2.size(); ++i)
    {
        vector<int> erroneous_data = data_with_crc_2;
        erroneous_data[i] ^= 1;

        if (check_for_errors(erroneous_data, crc_generator))
        {
            missed_errors++;
        }
        else
        {
            detected_errors++;
        }
    }

    cout << "\nError Detection Results";
    cout << "\nDetected Errors: " << detected_errors << endl;
    cout << "Missed Errors: " << missed_errors << endl;

    return 0;
}

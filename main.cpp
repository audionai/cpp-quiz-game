#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
void setColor(int color) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color); }
#endif
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>
using namespace std;
using namespace std::chrono;
struct Question {
    string questionText;
    string options[4];
    char correctAnswer; // 'A', 'B', 'C', or 'D'
};

char getTimedAnswer(int totalTimeSeconds, steady_clock::time_point quizStart) {
    char answer = '\0';
    auto now = steady_clock::now();

    while (duration_cast<seconds>(now - quizStart).count() < totalTimeSeconds) {
        if (cin.peek() != EOF) { // Check if input is available
            cin >> answer;
            return toupper(answer); // Return the answer in uppercase
        }

        now = steady_clock::now(); // Update the current time
        this_thread::sleep_for(milliseconds(100)); // Prevent tight looping
    }

    // Notify user of timeout
    cout << "\nTime's up for this question!\n";
    return '\0'; // Indicate no answer
}

void saveResultsAsJson(int score, int totalQuestions, int the_score, string username) {
    ofstream file("results.json", ios::app);
    file << "{\n";
    file << "  \"username\": \"" << username << "\",\n";
    file << "  \"score\": " << score << ",\n";
    file << "  \"totalQuestions\": " << totalQuestions << ",\n";
    file << "  \"status\": \"" << (the_score >= 50 ? "Passed" : "Failed") << "\",\n";
    time_t now = time(0);
    char* dt = ctime(&now);
    file << "  \"Date\": \"" << dt << "\",\n";
    file << "},\n";
    file.close();
}


// Function to load questions from a file
int loadQuestions(Question questions[], int maxQuestions) {
    ifstream file("question.txt");
    if (!file) {
        cerr << "Error: Unable to open file 'questions.txt'.\n";
        return 0;
    }

    int count = 0;
    while (count < maxQuestions && getline(file, questions[count].questionText)) { 
        for (int i = 0; i < 4; i++) {
            getline(file, questions[count].options[i]);
        }
        file >> questions[count].correctAnswer;
        file.ignore(); // Ignore the newline after the correct answer
        count++;
    }

    file.close();
    return count; // Return the number of questions loaded
}

// Function to present questions to the user and check correctness
int askQuestion(Question questions[], int totalQuestions) {
    auto rng = std::default_random_engine{std::random_device{}()};
    std::shuffle(questions, questions + totalQuestions, rng);

    int score = 0;
    auto quizStart = steady_clock::now(); // Start time for the quiz
    int totalTimeSeconds = 17; // 2 minutes for the entire quiz

    for (int i = 0; i < totalQuestions; i++) {
        cout << "\n" << questions[i].questionText << "\n";
        cout << "A) " << questions[i].options[0] << "\n";
        cout << "B) " << questions[i].options[1] << "\n";
        cout << "C) " << questions[i].options[2] << "\n";
        cout << "D) " << questions[i].options[3] << "\n";
        cout << "Your Answer: ";

        char answer = getTimedAnswer(totalTimeSeconds, quizStart);

        auto now = steady_clock::now();
        if (duration_cast<seconds>(now - quizStart).count() >= totalTimeSeconds) {
            setColor(10);
            cout << "🕰 Quiz ended due to timeout.\n";
            setColor(15);
            break;
        }

        if (answer == '\0') {
            cout << "Skipping question due to timeout.\n";
            continue;
        }

        while (answer != 'A' && answer != 'B' && answer != 'C' && answer != 'D') {
            cout << "Please enter A, B, C, or D: " << endl;;
            answer = getTimedAnswer(totalTimeSeconds, quizStart);
        }

        if (answer == questions[i].correctAnswer) {
            setColor(10); // Green for correct
            cout << "Correct!\n";
            score++;
        } else {
            setColor(4); // Red for incorrect
            cout << "Wrong! The correct answer was " << questions[i].correctAnswer << "\n";
        }
        setColor(15); // Reset to default
    }

    return score;
}
// Function to display the final score
void displayScore(int score, int totalQuestions, int the_score, string username) {
    cout << "\nYou scored " << score << " out of " << totalQuestions << "!\n";
    string random[4] ={"Don't give up.","You can do it!","You've got this!","I believe in you."};
    string result;
    int randomQ = rand() % 4;
    cout << "Username: " << username << endl;
    cout << "from 100 :" << the_score << endl;
    if (the_score < 50 ){
    cout << "Status Failed ,";
    cout << random[randomQ] << endl;
    } else {
    cout << "Status" << " Passed" << endl;
        result = "Passed";
    }
}

// Function to save the user's results to a file
void saveResults(int score, int totalQuestions,int the_score,string username) {
    ofstream file("results.txt", ios::app);
    if (!file) {
        cerr << "Error\n";
        return;
    }
    file << "Username: " << username << endl;
    file << "Score: " << score << " / " << totalQuestions << "\n";
    if (the_score >= 50){
    file << "Status: Passed" << endl;
    } else {
    file << "Status: Failed" << endl;
    }
    time_t now = time(0);
char* dt = ctime(&now);
file << "Date: " << dt;

    file.close();
    cout << "Your results have been saved to 'results.txt'.\n";
}




void displayLeaderboard() {
    ifstream file("results.txt");
    if (!file) {
        cout << "No leaderboard data available.\n";
        return;
    }

    string usernames[6]; // Array to store top 6 usernames
    int scores[6];        // Array to store top 6 scores
    int count = 0;

    // Initialize arrays with empty data to handle less than 6 players
    fill(begin(usernames), end(usernames), "");
    fill(begin(scores), end(scores), -1);

    string line;
    while (getline(file, line)) {
        if (line.find("Username: ") != string::npos) {
            string username = line.substr(line.find(": ") + 2);
            getline(file, line); // Skip "Score: "
            int score = stoi(line.substr(line.find(": ") + 2));

            // Insert the new username and score into the arrays if there's room
            if (count < 6) {
                usernames[count] = username;
                scores[count] = score;
                count++;
            } else {
                // If there are already 6 players, check if this new player has a higher score
                int minIndex = 0;
                for (int i = 1; i < 6; i++) {
                    if (scores[i] < scores[minIndex]) {
                        minIndex = i;
                    }
                }
                if (score > scores[minIndex]) {
                    usernames[minIndex] = username;
                    scores[minIndex] = score;
                }
            }
        }
    }

    file.close();

    // Display the top 6 players
    cout << "\n--- Top 6 Leaderboard ---\n";
    for (int i = 0; i < 6 && usernames[i] != ""; i++) {
        cout << i + 1 << ". " << usernames[i] << " - Score: " << scores[i] << "\n";
    }
}




void displayOptions(int score, int totalQuestions) { 
    cout << "\n Please choose from the following options: \n" << endl; 
    cout << "1) Display leaderboard" << endl; 
    if (score == totalQuestions) {
    } else {
    cout << "2) Play again" << endl; 
    }
    cout << "3) Exit" << endl; 
        
    cout << "4) Save result as JSON" << endl; 
}
void handleUserChoice(int score, int totalQuestions, int the_score, string username) {
    char retry;
    while (true) {
        displayOptions(score, totalQuestions);
    cin >> retry;
if (retry == '1') {
        displayLeaderboard();
} else if (retry == '2') {
    break;
} else if (retry == '3') {
    cout << "Good bye" << endl;
    exit(0);
} else if (retry == '4') {
        saveResultsAsJson(score, totalQuestions, the_score, username);
        cout << "Your results have beensaved to 'result.json'.\n";
} else {
    cout << "Invalid option" << endl;
    exit(0);
    }
}
}

void Adminonly(int score, int totalQuestions, int the_score, string username) {
    char adminnum;
    while (true) {
        cout << "\n--- Admin Menu ---\n";
        cout << "1) Add Questions\n";
        cout << "2) Exit Admin\n";
        cout << "Enter your choice: ";
        cin >> adminnum;

        if (adminnum == '1') {
            // Code to edit questions
            cout << "Editing questions...\n";

            // Open the questions file in append mode
            ofstream file("question.txt", ios::app);
            if (!file) {
                cerr << "Error: Unable to open 'question.txt'.\n";
                return;
            }

            string questionText, optionA, optionB, optionC, optionD, correctAnswer;
            cout << "Enter the question text: ";
            cin.ignore(); // Clear the input buffer
            getline(cin, questionText);
            cout << "Enter option A: ";
            getline(cin, optionA);
            cout << "Enter option B: ";
            getline(cin, optionB);
            cout << "Enter option C: ";
            getline(cin, optionC);
            cout << "Enter option D: ";
            getline(cin, optionD);
            cout << "Enter the correct answer (A/B/C/D): ";
            cin >> correctAnswer;

            // Write the new question to the file
            file << questionText << "\n";
            file << optionA << "\n";
            file << optionB << "\n";
            file << optionC << "\n";
            file << optionD << "\n";
            file << correctAnswer << "\n";

            file.close();
            cout << "Question added successfully!\n";
        } else if (adminnum == '2') {
            // Exit admin mode
            cout << "Exiting admin mode...\n";
            break;
        } else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}


int main() {
    setColor(15);
        srand(time(0));
        cout << "======================\n";
        cout << "Welcome to the Quiz Game!\n";
        cout << "======================\n";

    string username;
    cout << "What is your name: ";
    cin >> username;
    cout << "Password: ";
    string password;
    cin >> password;
    string adminPassword = "admin";
    string studentPassword = "stud";
    if ( password == adminPassword ) {
        Adminonly(0, 0, 0, username);
        return 0;
    }
    const int MAX_QUESTIONS = 10; // Fixed array size
    Question questions[MAX_QUESTIONS];

    int totalQuestions = loadQuestions(questions, MAX_QUESTIONS);

    if (totalQuestions == 0) {
        cerr << "No questions loaded. Exiting program.\n";
        return 1;
    }

    int score = askQuestion(questions, totalQuestions);

    int the_score = score * 100 / totalQuestions;
do {

    displayScore(score, totalQuestions, the_score, username);
    saveResults(score, totalQuestions, the_score,username);
handleUserChoice(score, totalQuestions, the_score ,username);
} while (true);
    return 0;
}

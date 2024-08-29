import pandas as pd
from sklearn.tree import DecisionTreeClassifier
from sklearn.model_selection import train_test_split
import joblib

# Generate or load the dataset
data = pd.read_csv('synthetic_dataset.csv')

def assign_risk_level(row):
    if row['Blood Oxygen (%)'] < 90 or row['Body Temperature (°C)'] > 37.5 or row['Heart Rate (bpm)'] > 100:
        return 'High'
    elif 90 <= row['Blood Oxygen (%)'] <= 95 or 37.0 <= row['Body Temperature (°C)'] <= 37.5 or 80 <= row['Heart Rate (bpm)'] <= 100:
        return 'Medium'
    else:
        return 'Low'

data['Risk Level'] = data.apply(assign_risk_level, axis=1)

# Prepare features and target
X = data[['Blood Oxygen (%)', 'Body Temperature (°C)', 'Heart Rate (bpm)']]
y = data['Risk Level']


# Train the Decision Tree Classifier
clf = DecisionTreeClassifier()
clf.fit(X, y)

# Save the trained model to a file
joblib.dump(clf, 'trained_model.pkl')



from flask import Flask, render_template, request, jsonify
import pandas as pd
import requests
import joblib

app = Flask(__name__)

CHANNEL_ID = '2575847'  
READ_API_KEY = '68J9D9S7SOZBT97R'  
NUM_RESULTS = 1000  

URL = f'https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds.json?api_key={READ_API_KEY}&results={NUM_RESULTS}'

# Load the pre-trained model
model = joblib.load('trained_model.pkl')

# Fetch the latest 30 data points
def fetch_data():
    response = requests.get(URL)
    if response.status_code == 200:
        data = response.json()
        df = pd.DataFrame(data['feeds'])
        df['created_at'] = pd.to_datetime(df['created_at'])
        df['field1'] = pd.to_numeric(df['field1'], errors='coerce')
        df['field2'] = pd.to_numeric(df['field2'], errors='coerce')
        df['field3'] = pd.to_numeric(df['field3'], errors='coerce')
        df['field4'] = pd.to_numeric(df['field4'], errors='coerce')
        df['field5'] = pd.to_numeric(df['field5'], errors='coerce')
        df['field6'] = pd.to_numeric(df['field6'], errors='coerce')
        df['field7'] = pd.to_numeric(df['field7'], errors='coerce')
        df['field8'] = pd.to_numeric(df['field8'], errors='coerce')
        

        df = df.rename(columns={
            'field1': 'Body Temperature (°C)',
            'field2': 'Heart Rate (bpm)',
            'field3': 'Blood Oxygen (%)',
            'field6': 'Atmospheric Temperature (°C)',
            'field7': 'Relative Humidity',
            'field8': 'Atmospheric Pressure'

        })
        df = df.dropna().sort_values(by='created_at').tail(30)
        return df
    else:
        return pd.DataFrame()


@app.route('/')
def index():
    data_df = fetch_data()
    
    if not data_df.empty:
        features=data_df[['Blood Oxygen (%)', 'Body Temperature (°C)', 'Heart Rate (bpm)']]
        predictions=model.predict(features)
        data_df['Risk Level']=predictions

        data = data_df.to_dict(orient='records')
    else:
        data = []
    
    return render_template('index.html', data=data)

if __name__ == "__main__":
    app.run(debug=True)

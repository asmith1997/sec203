from flask import Flask
from flask_restful import Api, Resource, reqparse
import mysql.connector
import base64
from werkzeug import datastructures, secure_filename
import os
import numpy as np
import numpy.random
from numpy import genfromtxt
import matplotlib.pyplot as plt
import time
from sklearn.neighbors import KNeighborsClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.linear_model import LogisticRegression
from sklearn.svm import SVC
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.neural_network import MLPRegressor
from sklearn.preprocessing import LabelEncoder
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
from matplotlib.colors import ListedColormap
#from sknn.mlp import Classifier, Layer
from sklearn.preprocessing import StandardScaler
from sklearn.datasets import make_moons, make_circles, make_classification
from PIL import Image
from imutils import paths
import argparse
import os
import pickle
from io import BytesIO
from skimage import img_as_float

app = Flask(__name__)
api = Api(app)

bioThreshold = -255.0
bioActivationThreshold = 1
modelPath = os.path.join(os.getcwd(), "files", "models")
heatmapPath = os.path.join(os.getcwd(), "files","heatmap")


labels = []
le = LabelEncoder()

def openConnection():
    #Connect to database on program start
    global connection
    connection = mysql.connector.connect(
        host="82.47.162.246",
        port="3306",
        user="geoff",
        passwd="posty",
        database="main"
    )
    #db.autocommit = True
    global cursor
    cursor = connection.cursor()

    
    #username = users[0]["username"]
    #password = users[0]["password"]
    #cursor.execute("SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED")
    #cursor.execute("INSERT INTO users(username, password_hash) VALUES(\"{}\", \"{}\")".format(username, password))
    #db.commit()
    

openConnection()
imgCount = 0
class Learner():
    def trainModel(self, username):
        imagePaths = paths.list_images(os.path.join(os.getcwd(), "files","heatmap"))
        global labels
        global le
        #Becomes 2D array of images as floats
        features = 0
        features = np.zeros([0, 0])
        
        labels = []
        imgCount = 0
        le = LabelEncoder()
        for imagePath in imagePaths:
            image = Image.open(imagePath)
            imageArr = img_as_float(np.array(image))
            imageArr.shape = (1, image.size[0] * image.size[1] * 4)
            if (imgCount == 0):
                features.shape = (0, image.size[0] * image.size[1] * 4)
            #print(imageArr.shape)
            try:
                features = np.append(features, imageArr, axis=0)
                label = imagePath.split(os.path.sep)[-2]
                
                labels.append(label)
                print("[IMGCOUNT]: {count}".format(count=imgCount))
                print("[LABEL]: {llabel}".format(llabel=label))
                #labels = le.fit_transform(labels).tolist()
                imgCount = imgCount + 1
            except Exception as e:
                #print(e)
                print("[WARNING] Image " + imagePath + "has invalid size and will be removed") # This is the image that is too big or mismatched to others.
                os.remove(imagePath)
                imgCount = imgCount-1
                continue
            
            #print(features.shape)



        labels = le.fit_transform(labels)
        print(labels)
        #print(labels)
        #labels.shape = (17,)

        
        #clf = MLPRegressor(solver="adam", alpha=1e-5, hidden_layer_sizes=(15,), random_state=9867, learning_rate="constant")#, activation="tanh")
        if (os.path.isfile(os.path.join(modelPath, username))):
            with open(os.path.join(modelPath, username), "rb") as inModel:
                userMLP = pickle.load(inModel)
            
        else:
            userMLP = MLPClassifier(hidden_layer_sizes=(15, ), activation="relu",
            solver="lbfgs", alpha=0.0001, batch_size="auto", learning_rate="constant", learning_rate_init=0.001, power_t=0.5,
            max_iter=200, shuffle=False, random_state=None, tol=0.001, verbose=False, warm_start=False,
            momentum=0.9, nesterovs_momentum=True, early_stopping=False, validation_fraction=0.1, beta_1=0.9, beta_2=0.999, epsilon=1e-08, n_iter_no_change=10)
        

        #Train model
        userMLP = userMLP.fit(features, labels)
        #userMLP.score(np.array())
        #f = open("models/{uname}".format(uname=username), "wb+")
        #pickle.dump(userMLP, f)
        with open(os.path.join(modelPath, username), "wb") as outModel:
            pickle.dump(userMLP, outModel)

        #userMLP.

        #userMLP.
        #Predict the user from the challenge heatmap
        #print(userMLP.predict(cHArr))
        #print("yay")
        
    def testHeatmap(self, challengeHeatmap, username):
        cH = Image.open(challengeHeatmap)
        print("[CHALLENGE]: "+ challengeHeatmap)
        cHArr = img_as_float(np.array(cH))
        
        cHArr.shape = (1, cH.size[0] * cH.size[1] * 4)
        #print(cHArr.shape)
        
        #os.remove(challengeHeatmap)
        if (os.path.isfile(os.path.join(modelPath, username))):
            with open(os.path.join(modelPath, username), "rb") as inModel:
                userMLP = pickle.load(inModel)
            
        else:
            return -1.0
        #Predict the user from the challenge heatmap
        #data = []
        #data.append(cHArr)
        #(trainX, testX, trainY, testY) = train_test_split (data, [0], test_size=0)



        result = userMLP.predict(cHArr)
        #for i in range(0,10,1):
        #    result = userMLP.predict(cHArr)
        #    print("[RESULT]: {res}".format(res=result))
        #    print("[RESULT LEN]: {res}".format(res=len(result)))

        #print("[SCORE]: {mlpscore}".format(mlpscore=userMLP.score(result, np.asarray(imgCount))))
        #arya = []
      
        trueUser = le.inverse_transform(result)
        
        #arya.append(0)
        print("[RESULT]: {res}".format(res=result))
        print("[ANSWER]: {ans}".format(ans=trueUser))
        #print("[RESULT LEN]: {res}".format(res=len(result)))
        #print("[SCORE]: {mlpscore}".format(mlpscore=userMLP.score(result, np.asarray(imgCount))))
        #print("[REPORT]:\n {score}".format(score=userMLP.score(cHArr, result))) #target_names=le.classes_)))
        if trueUser[0] == username:
            return True
        return False

class LoginUser(Resource):
    #Heatmap clamp values, affects the data range that can be represented
    elapseMin = 50.0
    elapseMax = 1500.0
    kdMin = 150.0
    kdMax = 300.0
    latencyMin = 300.0
    latencyMax = 1500.0
    
    #Permutes a value into a float ranging from 0.0 to 1.0 based on the min and max values.
    def permuteValue(self, value, satMin, satMax):
        value = self.clampValue(value, satMin, satMax)
        
        permutedVal = (value - satMin) / (satMax - satMin)

        return permutedVal

    def clampValue(self, value, satMin, satMax):
        if value < satMin:
            value = satMin
        elif value > satMax:
            value = satMax
        return value
    
    def getHeatmapData(self, args):
        file = args["file"]
        filename = secure_filename(file.filename)
        file.save(os.getcwd() + "\\files\\" + filename)
        
        if(filename.endswith("csv")):
            with open(os.getcwd() + "\\files\\" + filename) as openFile:
                #Read the received CSV into an ndarray.
                #data = genfromtxt(os.getcwd() + "\\files\\" + filename, delimiter=',')
                data = genfromtxt(openFile, delimiter=',')
                
                #Build the trigram data array
                trigrams = np.zeros([data.shape[0] - 2, 3], dtype=int)
                for i in range(0, data.shape[0] - 2):
                    #Set the trigram
                    #trigrams[i, 0] = i
                    #Set the elapse time (final key timestamp - initial key timestamp)
                    elapse = (data[i + 2, 1] - data[i, 1])
                    print(elapse, end='')
                    print(" ", end='')
                    #elapse = np.random.randint(self.elapseMin, self.elapseMax)
                    trigrams[i, 0] = self.permuteValue(elapse, self.elapseMin, self.elapseMax) * 255
                    #Set the keystroke duration
                    keystrokeDuration = data[i, 2] + data[i+1, 2] + data[i+2, 2]
                    print(keystrokeDuration, end='')
                    print(" ", end='')
                    #keystrokeDuration = np.random.randint(self.kdMin, self.kdMax)
                    trigrams[i, 1] = self.permuteValue(keystrokeDuration, self.kdMin, self.kdMax) * 255
                    #Set the latency
                    latency = data[i, 3] + data[i+1, 3] + data[i+2, 3]
                    print(latency, end='')
                    print(" ", end='')
                    #latency = np.random.randint(self.latencyMin, self.latencyMax)
                    trigrams[i, 2] = self.permuteValue(latency, self.latencyMin, self.latencyMax) * 255
                    print("\n")
            #os.remove(os.getcwd() + "\\files\\" + filename)
            return trigrams
        else:    
            #os.remove(os.getcwd() + "\\files\\" + filename)
            return -1


        
    def generateHeatmap(self, data, username = "unknown"):
        #Generate the heatmap from the extracted data.
        plt.axis('off')
        fig = plt.imshow(data, interpolation="nearest", cmap="seismic")


        #print(data)

        #Remove whitespace
        fig.axes.get_xaxis().set_visible(False)
        fig.axes.get_yaxis().set_visible(False)
        plt.margins(0, 0)
        plt.subplots_adjust(top=1, left=0, bottom=0, right=1, hspace=0, wspace=0)

        #Save the file (and create directory if it doesn't exist)
        if not os.path.exists(os.getcwd() + "\\files\\heatmap\\" + username):
            os.makedirs(os.getcwd() + "\\files\\heatmap\\" + username)
        filename = os.getcwd() + "\\files\\" + username + ".png"
        plt.savefig(filename, bbox_inches='tight', pad_inches=0)

        #Image is downscaled for efficiency.
        img = Image.open(filename)
        img = img.resize((16, 48))
        img.save(filename)
        return filename

    def userHasAmpleData(self, username):
        imagePaths = paths.list_images(os.path.join(os.getcwd(), "files","heatmap", username))
        
        if (len(list(imagePaths)) >= bioActivationThreshold):
            return True
        else:
            return False
        
    def post(self):
        userQuery = "SELECT username, password_hash FROM users WHERE users.username = %s"
        parser = reqparse.RequestParser()
        parser.add_argument("username", required=True)
        parser.add_argument("passHash", required=True)
        parser.add_argument("file", type=datastructures.FileStorage, location="files") 
        args = parser.parse_args()
        
        
        
        nameContainer = [args["username"]]
        cursor.execute(userQuery, nameContainer)
        user = cursor.fetchone()
        #user = 1
        #Attempt to authenticate user
        if user is None:
            return "User is inactive or does not exist", 404
        else:
            if args["passHash"] == user[1]:
                #Generate heatmap
                heatData = self.getHeatmapData(args)
                heatFilename = self.generateHeatmap(heatData, args["username"])

                if self.userHasAmpleData(args["username"]):
                    learner = Learner()
                    learner.trainModel(args["username"])
                    bioScore = learner.testHeatmap(heatFilename, args["username"])
                    print(bioScore)
                    if bioScore is True:
                        os.rename(heatFilename, heatmapPath + "/" + args["username"] + "/" + str(int(time.time())) + ".png")
                        return "User credentials accepted", 200
                    else:
                        return "Biometric auth rejection", 401
                else:
                    os.rename(heatFilename, heatmapPath + "/" + args["username"] + "/" + str(int(time.time())) + ".png")
                    return "User credentials accepted", 200
            else:
                return "Invalid credentials", 402
        
        return

class CreateUser(Resource):
    def post(self):
        userQuery = "INSERT INTO users(username, password_hash) VALUES (%s,%s)"
        parser = reqparse.RequestParser()
        parser.add_argument("username", required=True)
        parser.add_argument("passHash", required=True)
        args = parser.parse_args()

        #Attempt to create user
        try:
            argsContainer = [args["username"], args["passHash"]]
            cursor.execute(userQuery, argsContainer)
            connection.commit()
            return "User created successfully.", 201
        except mysql.connector.errors.IntegrityError:
            return "User could not be created. Does it already exist?", 409

api.add_resource(CreateUser, "/user/create")
api.add_resource(LoginUser, "/user/login")

app.run(debug=True, host="127.0.0.1", port=5000)

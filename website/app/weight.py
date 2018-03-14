from datetime import datetime
import pytz

def add_weight_point(weight):
    file_name = "weight"
    try:
        with open(file_name, 'a') as f:
            f.write(weight + ", ")
            tz = pytz.timezone('America/Denver')
            local_now = datetime.now(tz)
            dt_string = str(local_now.date()) + ' ' +  str(local_now.time())
            f.write(dt_string + "\n")
    except:
        print "Weight sad :-("
        pass

if __name__ == "__main__":
    add_weight_point("0.0")

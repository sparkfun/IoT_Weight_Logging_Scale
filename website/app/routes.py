from flask import render_template, send_from_directory
from app import app
from weight import add_weight_point
from plot_weight import plot_weight

@app.route('/')
@app.route('/index')
def index():
    return render_template('index.html', title='Home')

@app.route('/images/<path:path>')
def send_image(path):
    return send_from_directory('images', path)

@app.route('/post_weight/<string:weight>')
def post_weight(weight):
    add_weight_point(weight)
    plot_weight()
    return "weight posted"

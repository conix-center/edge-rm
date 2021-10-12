window_array = []

function reduce(data) {
	console.log("blah blah blah blah");
	console.log(data);
	console.log(data['value']);
	return data['value']
	window_array.push(data['value']);

	total = 0;
	for(var i = 0; i < window_array.length; i++) {
		total += window_array[i]
	}

	avg = total/window_array.length
	console.log(avg)
	return avg
}

module.exports = {reduce}
